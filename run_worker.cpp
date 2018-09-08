#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "definitions.h"

// TODO: turn this entire file into a class
std::string MACHINE_NAME = "blerg";

bool send_to_server(int sockfd, std::string msg) {
    if (DEBUG) { std::cout << "Sending: " << msg << std::endl; }
    size_t total_sent = 0;
    int bytes_sent;
    do {
        if ((bytes_sent = send(sockfd, msg.c_str(), msg.length(), 0)) == -1) {
            std::cerr << "send(): " << strerror(errno) << std::endl;
            return false;
        }
        total_sent += bytes_sent;
    } while (total_sent < msg.length());
    return true;
}

int client(std::string server_ip, std::string server_port,
        std::string map_exec, std::string reduce_exec,
        int wait_time = 10) {
    struct addrinfo hints;
    struct addrinfo *servinfo;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int status;
    if ((status = getaddrinfo(server_ip.c_str(), server_port.c_str(), &hints, &servinfo)) != 0) {
        std::cerr << "getaddrinfo(): " << gai_strerror(status) << std::endl;
        return 1;
    }

    int sockfd;
    struct addrinfo *p;
    for (p = servinfo; p != nullptr; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            std::cerr << "socket(): " << strerror(errno) << std::endl;
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            std::cerr << "connect(): " << strerror(errno) << std::endl;
            continue;
        }

        break;
    }

    if (p == nullptr) {
        std::cerr << "err: failed to connect" << std::endl;
        return 1;
    }

    // Tell the server that we connected for the first time.
    if (!send_to_server(sockfd, CONNECT_MSG)) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }
    if (!send_to_server(sockfd, MACHINE_NAME)) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }
    if (!send_to_server(sockfd, END_MSG)) {
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }

    if (DEBUG) { std::cout << "sent initial connect message to server." << std::endl; }

    // wait for first task from the master and then execute it
    std::string master_data;
    constexpr int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int bytes_received;
    do {
        if ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) == -1) {
            std::cerr << "recv(): " << strerror(errno);
            break;
        }

        // master_data contains the task name at this point
        master_data.append(buffer, bytes_received);
    } while (bytes_received > 0);

    close(sockfd);

    if (DEBUG) { std::cout << "received task from master." << std::endl; }

    // in a loop: complete the task, tell the server that the task is completed,
    // get a new task (or a message)
    do {
        // complete the current task (stored in master_data)
        if (master_data == WAIT_MSG) {
            sleep(wait_time);
        } else if (master_data == DISCONNECT_MSG) {
            close(sockfd);
            break;
        } else {
            std::ifstream task_file(master_data);
            if (master_data.find(map_in_splits) != std::string::npos) {
                // Map tasks. kv_filename is the path to a file that contains one filepath in each
                // line. Each filepath within this file is the argument to a call of ./maptask
                if (DEBUG) { std::cout << "map task called on: " << master_data << std::endl; }
                std::string kv_filename;
                while (std::getline(task_file, kv_filename)) {
                    int status;
                    pid_t pid = fork();
                    if (pid != 0) {     // this is the parent process
                        wait(&status);
                    } else {            // this is the child process
                        execl(map_exec.c_str(), map_exec.c_str(), kv_filename.c_str(), (char *) NULL);
                    }
                }
            } else if (master_data.find(key_groups) != std::string::npos) {
                // Reduce tasks. kv_filename is the path to a file. This filepath should be the
                // argument to ./reducetask. Note that this differs from the map task case in that
                // for map tasks, several "tasks" are sent by the server together as a batch. For
                // reduce tasks, only 1 singleton key (with its values) is sent to a worker at a
                // time.
                if (DEBUG) { std::cout << "reduce tasked called on: " << master_data << std::endl; }
                int status;
                pid_t pid = fork();
                if (pid != 0) {     // this is the parent process
                    wait(&status);
                } else {            // this is the child process
                    execl(reduce_exec.c_str(), reduce_exec.c_str(), master_data.c_str(), (char *) NULL);
                }
            }
            task_file.close();
        }

        if (DEBUG) { std::cout << "current task completed." << std::endl; }

        // connect to server again to say that task is finished
        for (p = servinfo; p != nullptr; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                std::cerr << "socket(): " << strerror(errno) << std::endl;
                continue;
            }

            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                std::cerr << "connect(): " << strerror(errno) << std::endl;
                continue;
            }

            break;
        }

        if (p == nullptr) {
            std::cerr << "err: failed to connect" << std::endl;
            return 1;
        }

        // Tell the server that the task is finished.
        if (!send_to_server(sockfd, FIN_TASK_MSG)) {
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }
        if (!send_to_server(sockfd, MACHINE_NAME)) {
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }
        if (!send_to_server(sockfd, END_MSG)) {
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }

        // get a new task from the server (or the message that everything is done
        if (DEBUG) { std::cout << "waiting for another task from server." << std::endl; }
        master_data.clear();
        do {
            if ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // master_data contains the task name at this point
            master_data.append(buffer, bytes_received);
        } while (bytes_received > 0);

        close(sockfd);
    } while (true);

    freeaddrinfo(servinfo);
    return 0;
}

int main(int argc, char* argv[]) {
    // TODO: add parameters
    client("127.0.0.1", "8000", "./maptask", "./reducetask");
}
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

    // Leave this commented out. We need servinfo later.
    // freeaddrinfo(servinfo);

    // Tell the server that we connected for the first time.
    int total_sent = 0;
    int bytes_sent;
    do {
        if ((bytes_sent = send(sockfd, CONNECT_MSG, CONNECT_MSG_LEN, 0)) == -1) {
            std::cerr << "send(): " << strerror(errno) << std::endl;
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }
        total_sent += bytes_sent;
    } while (total_sent < CONNECT_MSG_LEN);

    std::cout << "sent initial connect message to server." << std::endl;

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
        master_data.append(buffer, BUFFER_SIZE);
    } while (bytes_received > 0);

    close(sockfd);

    std::cout << "received task from master." << std::endl;

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
        master_data.clear();

        std::cout << "current task completed." << std::endl;

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
        total_sent = 0;
        do {
            if ((bytes_sent = send(sockfd, FIN_TASK_MSG, FIN_TASK_MSG_LEN, 0)) == -1) {
                std::cerr << "send(): " << strerror(errno) << std::endl;
                close(sockfd);
                freeaddrinfo(servinfo);
                return 1;
            }
            total_sent += bytes_sent;
        } while (total_sent < FIN_TASK_MSG_LEN);

        // get a new task from the server (or the message that everything is done
        std::cout << "waiting for another task from server." << std::endl;
        do {
            if ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // master_data contains the task name at this point
            master_data.append(buffer, BUFFER_SIZE);
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
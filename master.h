#ifndef MASTER_H
#define MASTER_H

#include <cerrno>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"
#include "machine.h"
#include "maptask.h"

template <typename Kin, typename Vin,
          typename Kinter, typename Vinter,
          typename Kout, typename Vout>
class Master {
private:
    static const int QUEUE_LENGTH = 10;
    static const int BUFFER_SIZE = 4096;
    std::vector<char> buffer;

    const std::string server_port;
    std::set<Machine> workers;

    Phase phase;

    std::map<MapTask<Kin, Vin, Kinter, Vinter>, TaskStatus> map_task_status;
    std::map<MapTask<Kin, Vin, Kinter, Vinter>, Machine> map_task_assignments;

    std::map<ReduceTask, TaskStatus> reduce_task_status;
    std::map<ReduceTask, Machine> reduce_task_assignments;
public:
    Master(std::string server_port) :
            buffer(BUFFER_SIZE),
            server_port(server_port),
            phase(Phase::map_phase) {}

    int Master::start_server() {
        struct addrinfo hints;
        struct addrinfo *servinfo;

        std::memset(&hints, 0, sizeof(hints));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;

        int status;
        if ((status = getaddrinfo(NULL, server_port.c_str(), &hints, &servinfo)) != 0) {
            std::cerr << "getaddrinfo(): " << gai_strerror(status) << std::endl;
            return 1;
        }

        int sockfd;
        if ((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
            std::cerr << "socket(): " << strerror(errno) << std::endl;
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }
        if (bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
            std::cerr << "bind(): " << strerror(errno) << std::endl;
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }
        if (listen(sockfd, QUEUE_LENGTH) == -1) {
            std::cerr << "listen(): " << strerror(errno) << std::endl;
            close(sockfd);
            freeaddrinfo(servinfo);
            return 1;
        }

        struct sockaddr_storage client_addr;
        socklen_t addr_size = sizeof(client_addr);
        int client_fd;
        while (true) {
            if ((client_fd = accept(sockfd, (struct sockaddr *) &client_addr, &addr_size)) == -1) {
                std::cerr << "accept(): " << strerror(errno) << std::endl << "Skipping." << std::endl;
                continue;
            }

            std::string worker_data;
            int bytes_received;
            do {
                if ((bytes_received = recv(client_fd, &buffer[0], buffer.size(), 0)) == -1) {
                    std::cerr << "recv(): " << strerror(errno);
                    break;
                }

                // in the future, probably should write to a file
                worker_data.append(buffer.cbegin(), buffer.cend());
            } while (bytes_received > 0);

            char client_host[1024];
            char client_service[20];
            if (status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
                std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
                continue;
            }

            /* do stuff with the data here */
            if (worker_data == connect_msg) {
                Machine client(client_host, client_service);
                workers.insert(client);
            }
            std::cout << worker_data;
            
            close(client_fd);
        }

        close(sockfd);
        freeaddrinfo(servinfo);
    }
};

#endif
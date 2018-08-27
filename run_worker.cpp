#include <cerrno>
#include <cstring>
#include <iostream>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"

int client(std::string server_ip, std::string server_port) {
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

    freeaddrinfo(servinfo);

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
    } while (total_sent < CONNECT_MSG_LEN);

    // wait for tasks/commands from the master and then execute them
    while (true) {
        std::string master_data;
        constexpr int BUFFER_SIZE = 4096;
        char buffer[BUFFER_SIZE];
        int bytes_received;
        do {
            if ((bytes_received = recv(sockfd, buffer, BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // in the future, probably should write to a file
            master_data.append(buffer, BUFFER_SIZE);
        } while (bytes_received > 0);

    }

    close(sockfd);

    return 0;
}

int main() {
    client("127.0.0.1", "8000");
}
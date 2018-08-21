#include <cerrno>
#include <cstring>
#include <iostream>

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

    // begin sending data or smth
    int bytes_sent;
    if ((bytes_sent = send(sockfd, connect_msg, connect_msg_len, 0)) == -1) {
        std::cerr << "send(): " << strerror(errno) << std::endl;
        close(sockfd);
        freeaddrinfo(servinfo);
        return 1;
    }
    
    // end send data

    close(sockfd);

    return 0;
}

int main() {
    client("127.0.0.1", "8000");
}
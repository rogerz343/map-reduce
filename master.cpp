#include "master.h"

Master::Master(std::string server_port) : server_port(server_port) {

}

int Master::start_server(std::string server_port) {
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
            std::cerr << "accept(): " << strerror(errno) << std::endl;
            continue;
        }

        std::string worker_data;
        int bytes_received;
        do {
            if ((bytes_received = recv(client_fd, &buffer[0], buffer.length(), 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // in the future, probably should write to a file
            worker_data.append(buffer.cbegin(), buffer.cend());
        } while (bytes_received > 0);

        /* do stuff with the data here */
        cout << worker_data;
        
        close(client_fd);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
}
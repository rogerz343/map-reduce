#include "master.h"

Master::Master(std::string master_name,
        std::string server_port,
        std::vector<std::string> input_files,
        int num_splits) :
        buffer(BUFFER_SIZE),
        master_name(master_name),
        server_port(server_port),
        phase(Phase::map_phase) {
    int split_size = input_files.size() / num_splits;
    for (int i = 0; i < num_splits; ++i) {
        std::string split_name = master_name + "_split" + std::to_string(i);
        std::ofstream split_file(split_name, std::ios::trunc);
        for (int j = 0; j < split_size; ++j) {
            std::string input_filename = input_files.back();
            std::ifstream input_file(input_filename);
            input_files.pop_back();

            split_file << input_filename;
            split_file << input_file.rdbuf();
        }
    }
}

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

        if (worker_data == CONNECT_MSG) {
            Machine client(client_host, client_service);
            workers.insert(client);

            // send a task to the worker
        }
        
        close(client_fd);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
}
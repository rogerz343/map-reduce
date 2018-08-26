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
        std::string split_name = master_name + "_map_split" + std::to_string(i);
        std::ofstream split_file(split_name, std::ios::trunc);
        for (int j = 0; j < split_size; ++j) {
            std::string input_filename = input_files.back();
            input_files.pop_back();
            std::ifstream input_file(input_filename);

            split_file << input_filename << DELIMITER_NEWLINE;
            split_file << input_file.rdbuf() << DELIMITER_NEWLINE;

            input_file.close();
        }

        // remainder of files go in last task
        if (i == num_splits - 1) {
            while (!input_files.empty()) {
                std::string input_filename = input_files.back();
                input_files.pop_back();
                std::ifstream input_file(input_filename);

                split_file << input_filename << DELIMITER_NEWLINE;
                split_file << input_file.rdbuf() << DELIMITER_NEWLINE;

                input_file.close();
            }
        }

        split_file.close();
        map_task_statuses[split_name] = TaskStatus::unassigned;
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
        if ((status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
            std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
            continue;
        }

        if (worker_data == CONNECT_MSG) {
            Machine client(client_host, client_service);
            workers.insert(client);

            // send a task to the worker
            Task t;
            if (phase == Phase::map_phase) {
                for (std::pair<const Task, TaskStatus> &kv : map_task_statuses) {
                    if (kv.second == TaskStatus::unassigned) {
                        t = kv.first;
                        break;
                    }
                }
                if (!t.empty()) {
                    map_task_statuses[t] = TaskStatus::in_progress;
                    map_task_assignments[t] = client;
                }
            } else if (phase == Phase::reduce_phase) {
                for (std::pair<const Task, TaskStatus> &kv : reduce_task_statuses) {
                    if (kv.second == TaskStatus::unassigned) {
                        t = kv.first;
                        break;
                    }
                }
                if (!t.empty()) {
                    reduce_task_statuses[t] = TaskStatus::in_progress;
                    reduce_task_assignments[t] = client;
                }
            } else {
                // this should never happen; remove this code after confident
                std::cerr << "should never happen: new connection after reduce done";
            }
            // send the task file over
            if (!t.empty()) {
                std::ifstream task_file(t);
                
            }
            
        }
        
        close(client_fd);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
    return 0;
}
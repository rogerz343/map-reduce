#include "master.h"

/**
 * master_name: an identifying name for the task.
 * server_port: the port to host the server on
 * input_files: a list of the input files for the task.
 * num_splits: the number of parts in the partition the input_files
 * 
 * This constructor creates num_splits files, each of which represents one batch of tasks to send to
 * a worker. Each of these files contains the names of the input_files in that batch of inputs, one
 * on each line. Each of the split files are put into the corresponding directory (see
 * definitions.h) and have the suffix "_map_split#" where # is a number.
 */
Master::Master(std::string master_name,
        std::string server_port,
        std::vector<std::string> input_files,
        int num_splits) :
        master_name(master_name),
        server_port(server_port),
        phase(Phase::map_phase) {
    int split_size = input_files.size() / num_splits;
    for (int i = 0; i < num_splits; ++i) {
        std::string split_name = map_in_splits + master_name + "_map_in_split" + std::to_string(i);
        std::ofstream split_file(split_name, std::ios::trunc);
        for (int j = 0; j < split_size; ++j) {
            std::string input_filename = input_files.back();
            input_files.pop_back();
            split_file << input_filename << std::endl;
        }

        // remainder of files go in last task
        if (i == num_splits - 1) {
            while (!input_files.empty()) {
                std::string input_filename = input_files.back();
                input_files.pop_back();
                split_file << input_filename << std::endl;
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

        // read bytes and communicate back and forth until client closes connection
        bool errorflag = false;
        bool done = false;
        std::string worker_data;
        int bytes_received;
        do {
            if ((bytes_received = recv(client_fd, recv_buffer, RECV_BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // in the future, probably should write to a file; or not idk
            worker_data.append(recv_buffer, bytes_received);

            if (worker_data.find(CONNECT_MSG) == 0) {
                worker_data.erase(0, CONNECT_MSG_LEN);

                // get info about the message sender
                char client_host[1024];
                char client_service[20];
                if ((status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
                    std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
                    errorflag = true;
                    continue;
                }

                Machine client(client_host, client_service);
                workers.insert(client);

                // Pick an unassigned task (file) to send to the worker
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
                    size_t total_sent = 0;
                    int bytes_sent;
                    do {
                        if ((bytes_sent = send(sockfd, t.c_str(), t.length(), 0)) == -1) {
                            std::cerr << "send(): " << strerror(errno) << std::endl;
                            close(sockfd);
                            freeaddrinfo(servinfo);
                            return 1;
                        }
                    } while (total_sent < t.length());
                } else {
                    // TODO
                    // no tasks found; this should never happen (maybe?)
                }

                std::cout << "sent task to worker." << std::endl;
                done = true;
                close(client_fd);
            }

            if (worker_data.find(FIN_TASK_MSG) == 0) {
                worker_data.erase(0, FIN_TASK_MSG_LEN);

                // get info about the message sender
                char client_host[1024];
                char client_service[20];
                if ((status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
                    std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
                    errorflag = true;
                    continue;
                }

                // Machine client(client_host, client_service);
                // workers.insert(client);

                // TODO:
                // do stuff with the finished map or reduce task
            }

        } while (!errorflag && !done && bytes_received > 0);

        close(client_fd);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
    return 0;
}
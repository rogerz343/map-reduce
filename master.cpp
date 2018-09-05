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
    while (phase != Phase::finished_phase) {
        if ((client_fd = accept(sockfd, (struct sockaddr *) &client_addr, &addr_size)) == -1) {
            std::cerr << "accept(): " << strerror(errno) << std::endl << "Skipping." << std::endl;
            continue;
        }

        std::cout << "connected to a client." << std::endl;

        // read bytes and communicate back and forth until client closes connection
        bool errorflag = false;
        bool done_with_client = false;
        std::string worker_data;
        int bytes_received;
        do {
            if ((bytes_received = recv(client_fd, recv_buffer, RECV_BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            // in the future, probably should write to a file; or not idk
            worker_data.append(recv_buffer, bytes_received);

            // New worker connected: assign it a task.
            if (worker_data.find(CONNECT_MSG) == 0) {
                worker_data.erase(0, CONNECT_MSG_LEN);

                std::cout << "New worker has connected." << std::endl;

                // get info about the message sender
                char client_host[1024];
                char client_service[20];
                if ((status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
                    std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
                    errorflag = true;
                    continue;
                }

                Machine client(client_host, client_service);
                workers[client] = MachineStatus::idle;

                // Pick an unassigned task (filepath/name) to send to the worker
                Task t = assign_task(client);

                // send the task file over
                if (!t.empty()) {
                    if (!send_to_client(client_fd, t)) {
                        close(client_fd);
                        freeaddrinfo(servinfo);
                        return 1;
                    }
                    std::cout << "sent task to worker." << std::endl;
                } else {
                    // no tasks found.
                    // TODO: idk what to do
                }
                done_with_client = true;
                close(client_fd);
            }

            // Worker has finished task. Record this and give the worker a new task (if applicable).
            if (worker_data.find(FIN_TASK_MSG) == 0) {
                worker_data.erase(0, FIN_TASK_MSG_LEN);

                std::cout << "Worker has finished a task." << std::endl;

                // get info about the message sender
                char client_host[1024];
                char client_service[20];
                if ((status = getnameinfo((sockaddr *) &client_addr, addr_size, client_host, sizeof(client_host), client_service, sizeof(client_service), 0)) != 0) {
                    std::cerr << "getnameinfo(): " << gai_strerror(status) << std::endl << "Skipping." << std::endl;
                    errorflag = true;
                    continue;
                }

                // Record task as being finished
                Machine client(client_host, client_service);
                Task finished_task = map_machine_assignments[client];
                map_task_statuses[finished_task] = TaskStatus::finished;
                map_task_assignments.erase(finished_task);
                map_machine_assignments.erase(client);

                // Find a new task to assign to worker.
                Task t = assign_task(client);

                // Send the new task to the worker.
                if (!t.empty()) {
                    if (!send_to_client(client_fd, t)) {
                        close(client_fd);
                        freeaddrinfo(servinfo);
                        return 1;
                    }
                } else {
                    // No tasks found; current phase is done.
                    std::cout << "All current tasks completed. Moving to next phase." << std::endl;
                    if (phase == Phase::map_phase) {
                        group_keys();
                        phase = Phase::reduce_phase;
                    } else if (phase == Phase::reduce_phase) {
                        phase = Phase::finished_phase;
                    }
                }

                done_with_client = true;
                close(client_fd);
            }

        } while (!errorflag && !done_with_client && bytes_received > 0);

        std::cout << "closing connection to client." << std::endl;
        close(client_fd);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
    return 0;
}

Task Master::assign_task(const Machine &client) {
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
            map_machine_assignments[client] = t;
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
            reduce_machine_assignments[client] = t;
        }
    } else {
        // this should never happen; remove this code after confident
        std::cerr << "should never happen: new connection after reduce done";
    }
    return t;
}

bool Master::send_to_client(int client_fd, std::string msg) {
    size_t total_sent = 0;
    int bytes_sent;
    do {
        if ((bytes_sent = send(client_fd, msg.c_str(), msg.length(), 0)) == -1) {
            std::cerr << "send(): " << strerror(errno) << std::endl;
            return false;
        }
        total_sent += bytes_sent;
    } while (total_sent < msg.length());
    return true;
}

bool Master::group_keys() {
    std::cout << "group_keys() called." << std::endl;

    std::unordered_map<std::string, std::vector<std::string>> intermediate_keys;

    DIR *map_out_dir;
    struct dirent *file;
    if ((map_out_dir = opendir(map_out)) != NULL) {
        while ((file = readdir(map_out_dir)) != NULL) {
            std::string filename = file->d_name;
            if (filename == "." || filename == ".." || filename ==  "placeholder.txt") { continue; }
            std::string filepath = map_out + filename;
            std::ifstream kv_file(filepath);
            std::string key;
            std::getline(kv_file, key, DELIMITER_NEWLINE);
            kv_file.close();

            // TODO: THIS FUNCTION IS BROKEN

            if (intermediate_keys.find(key) == intermediate_keys.end()) {
                intermediate_keys[key] = std::vector<std::string>();
            }
            intermediate_keys[key].push_back(filepath);
        }
        closedir(map_out_dir);

        for (auto &entry : intermediate_keys) {
            std::cout << entry.first << std::endl;
            std::ofstream output_file(key_groups + entry.first, std::ios::trunc);
            if (!output_file.is_open()) { std::cout << "FOEIWF" << std::endl; return false; }

            std::vector<std::string> &vals = entry.second;
            for (const std::string &val : vals) {
                output_file << val << std::endl;
            }
            output_file.close();
        }
        std::cout << "group_keys() ran successfully." << std::endl;
        return true;
    } else {
        std::cout << "group_keys(): fatal: unable to open directory" << std::endl;
        return false;
    }
}

// TODO: U WERE WORKING ON THIS; YOU LITERALLY JUST COPY PASTA THE READ-FROM-DIRECTORY
// CODE. CHANGE IT TO READ FROM intermediate_out
bool Master::start_reduce_phase() {
    DIR *map_out_dir;
    struct dirent *file;
    if ((map_out_dir = opendir(map_out)) != NULL) {
        while ((file = readdir(map_out_dir)) != NULL) {
            std::string filename = file->d_name;
            if (filename == "." || filename == ".." || filename ==  "placeholder.txt") { continue; }
            std::string filepath = map_out + filename;
            std::ifstream kv_file(filepath);
            std::string key;
            std::getline(kv_file, key, DELIMITER_NEWLINE);
            kv_file.close();

            if (intermediate_keys.find(key) == intermediate_keys.end()) {
                intermediate_keys[key] = std::vector<std::string>();
            }
            intermediate_keys[key].push_back(filepath);
        }
        closedir(map_out_dir);

        for (auto &entry : intermediate_keys) {
            std::ofstream output_file(key_groups + entry.first, std::ios::trunc);
            if (!output_file.is_open()) { return false; }

            std::vector<std::string> &vals = entry.second;
            for (const std::string &val : vals) {
                output_file << val << std::endl;
            }
            output_file.close();
        }
        std::cout << "group_keys() ran successfully." << std::endl;
        return true;
    } else {
        std::cout << "group_keys(): fatal: unable to open directory" << std::endl;
        return false;
    }
}
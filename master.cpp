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
    map_tasks[TaskStatus::unassigned] = std::unordered_set<Task>();
    map_tasks[TaskStatus::in_progress] = std::unordered_set<Task>();
    map_tasks[TaskStatus::finished] = std::unordered_set<Task>();
    reduce_tasks[TaskStatus::unassigned] = std::unordered_set<Task>();
    reduce_tasks[TaskStatus::in_progress] = std::unordered_set<Task>();
    reduce_tasks[TaskStatus::finished] = std::unordered_set<Task>();
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
        map_tasks[TaskStatus::unassigned].insert(split_name);
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

        if (DEBUG) { std::cout << "connected to a client." << std::endl; }

        // read bytes and communicate back and forth until server or client closes connection
        std::string worker_data;
        int bytes_received;
        do {
            if ((bytes_received = recv(client_fd, recv_buffer, RECV_BUFFER_SIZE, 0)) == -1) {
                std::cerr << "recv(): " << strerror(errno);
                break;
            }

            worker_data.append(recv_buffer, bytes_received);

            // If new worker connected, assign it a task.
            if (worker_data.rfind(END_MSG) != std::string::npos
                    && worker_data.find(CONNECT_MSG) != std::string::npos) {
                worker_data.erase(0, CONNECT_MSG_LEN);
                worker_data.erase(worker_data.length() - END_MSG_LEN, END_MSG_LEN);

                // worker_data now contains worker's machine name
                if (!handle_new_connection(client_fd, worker_data)) {
                    close(client_fd);
                    freeaddrinfo(servinfo);
                    return 1;
                }
                close(client_fd);
                break;
            }

            // Worker has finished task. Record this and give the worker a new task (if applicable).
            if (worker_data.rfind(END_MSG) != std::string::npos
                    && worker_data.find(FIN_TASK_MSG) != std::string::npos) {
                worker_data.erase(0, FIN_TASK_MSG_LEN);
                worker_data.erase(worker_data.length() - END_MSG_LEN, END_MSG_LEN);

                // worker_data now contains worker's machine name
                if (!handle_fin_task(client_fd, worker_data)) {
                    close(client_fd);
                    freeaddrinfo(servinfo);
                    return 1;
                }
                close(client_fd);
                break;
            }
        } while (bytes_received > 0);
    }

    close(sockfd);
    freeaddrinfo(servinfo);
    
    std::cout << "Master has finished executing. Exiting." << std::endl;
    return 0;
}

bool Master::handle_new_connection(int client_fd, const Machine &client) {
    if (DEBUG) { std::cout << "New worker has connected." << std::endl; }

    // Pick an unassigned Task (filepath/name) to send to the worker
    Task t = assign_task(client);
    if (!t.empty()) {
        if (!send_to_client(client_fd, t)) { return false; }
    } else {
        // No tasks to do currently
        if (phase == Phase::map_phase) {
            if (map_tasks[TaskStatus::in_progress].empty()) {
                // Map phase done. Move on to grouping keys and reduce phase.
                std::cout << "All map tasks completed. Moving to next phase." << std::endl;
                group_keys();
                start_reduce_phase();

                t = assign_task(client);
                if (!t.empty()) {
                    if (!send_to_client(client_fd, t)) { return false; }
                } else {
                    // No tasks found in reduce phase; nothing left for this machine to do.
                    phase = Phase::finished_phase;
                    if (!send_to_client(client_fd, DISCONNECT_MSG)) { return false; }
                }
            } else {
                // Map phase still running on other machines. Tell client to come back
                // later to hopefully get a reduce task.
                if (!send_to_client(client_fd, WAIT_MSG)) { return false; }
            }
        } else if (phase == Phase::reduce_phase) {
            // No tasks found in reduce phase; nothing left for this machine to do.
            if (!send_to_client(client_fd, DISCONNECT_MSG)) { return false; }
        }
    }
    return true;
}

bool Master::handle_fin_task(int client_fd, const Machine &client) {
    if (DEBUG) { std::cout << "Worker has finished a task." << std::endl; }

    // Record task as being finished
    if (phase == Phase::map_phase) {
        Task finished_task = map_machine_assignments[client];
        map_tasks[TaskStatus::in_progress].erase(finished_task);
        map_tasks[TaskStatus::finished].insert(finished_task);
        map_task_assignments.erase(finished_task);
        map_machine_assignments.erase(client);
    } else if (phase == Phase::reduce_phase) {
        Task finished_task = reduce_machine_assignments[client];
        reduce_tasks[TaskStatus::in_progress].erase(finished_task);
        reduce_tasks[TaskStatus::finished].insert(finished_task);
        reduce_task_assignments.erase(finished_task);
        reduce_machine_assignments.erase(client);
    }

    // Find a new task to assign to worker.
    Task t = assign_task(client);
    if (!t.empty()) {
        if (!send_to_client(client_fd, t)) { return false; }
    } else {
        // No tasks found.
        if (phase == Phase::map_phase) {
            if (map_tasks[TaskStatus::in_progress].empty()) {
                // Map phase done. Move on to grouping keys and reduce phase.
                if (DEBUG) { std::cout << "All map tasks completed. Moving to next phase." << std::endl; }
                group_keys();
                start_reduce_phase();

                t = assign_task(client);
                if (!t.empty()) {
                    if (!send_to_client(client_fd, t)) { return false; }
                } else {
                    // No tasks found in reduce phase; nothing left for this machine to do.
                    if (!send_to_client(client_fd, DISCONNECT_MSG)) { return false; }
                }
            } else {
                // Map phase still running on other machines. Tell client to come back
                // later to hopefully get a reduce task.
                if (!send_to_client(client_fd, WAIT_MSG)) { return false; }
            }
        } else if (phase == Phase::reduce_phase) {
            // No tasks found in reduce phase; nothing left for this machine to do.
            if (!send_to_client(client_fd, DISCONNECT_MSG)) { return false; }
        }
    }
    return true;
}

Task Master::assign_task(const Machine &client) {
    Task t;
    if (phase == Phase::map_phase) {
        std::unordered_set<Task> &unassigned = map_tasks[TaskStatus::unassigned];
        if (!unassigned.empty()) {
            t = *(unassigned.begin());
            unassigned.erase(t);
            map_tasks[TaskStatus::in_progress].insert(t);
            map_task_assignments[t] = client;
            map_machine_assignments[client] = t;
        }
    } else if (phase == Phase::reduce_phase) {
        std::unordered_set<Task> &unassigned = reduce_tasks[TaskStatus::unassigned];
        if (!unassigned.empty()) {
            t = *(unassigned.begin());
            unassigned.erase(t);
            reduce_tasks[TaskStatus::in_progress].insert(t);
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
    if (DEBUG) { std::cout << "Sending: " << msg << std::endl; }
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
    if (DEBUG) { std::cout << "group_keys() called." << std::endl; }

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

            if (intermediate_keys.find(key) == intermediate_keys.end()) {
                intermediate_keys[key] = std::vector<std::string>();
            }
            intermediate_keys[key].push_back(filepath);
        }
        closedir(map_out_dir);

        for (auto &entry : intermediate_keys) {
            std::ofstream output_file(key_groups + entry.first, std::ios::trunc);
            if (!output_file.is_open()) { std::cout << "FOEIWF" << std::endl; return false; }

            std::vector<std::string> &vals = entry.second;
            for (const std::string &val : vals) {
                output_file << val << std::endl;
            }
            output_file.close();
        }
        if (DEBUG) { std::cout << "group_keys() ran successfully." << std::endl; }
        return true;
    } else {
        std::cout << "group_keys(): fatal: unable to open directory" << std::endl;
        return false;
    }
}

bool Master::start_reduce_phase() {
    if (DEBUG) { std::cout << "start_reduce_phase() called." << std::endl; }
    std::unordered_set<Task> &unassigned = reduce_tasks[TaskStatus::unassigned];
    DIR *key_groups_dir;
    struct dirent *file;
    if ((key_groups_dir = opendir(key_groups)) != NULL) {
        while ((file = readdir(key_groups_dir)) != NULL) {
            std::string filename = file->d_name;
            if (filename == "." || filename == ".." || filename ==  "placeholder.txt") { continue; }
            std::string filepath(key_groups + filename);
            unassigned.insert(filepath);
        }
        closedir(key_groups_dir);
        phase = Phase::reduce_phase;
        return true;
    } else {
        std::cout << "start_reduce_phase(): fatal: unable to open directory" << std::endl;
        return false;
    }
}
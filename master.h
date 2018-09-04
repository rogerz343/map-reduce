/**
 * master.h
 * 
 * input_files should be a list of filenames, where each file represents one key-value input pair.
 * The filename will be used as the key, and the contents of the file will be used as the value.
 * 
 */

#ifndef MASTER_H
#define MASTER_H

#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <vector>

#include <dirent.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"
#include "machine.h"

class Master {
private:
    static const int QUEUE_LENGTH = 10;
    static const int RECV_BUFFER_SIZE = 4096;
    char recv_buffer[RECV_BUFFER_SIZE];
    // static const int SEND_BUFFER_SIZE = 4096;
    // char send_buffer[SEND_BUFFER_SIZE];

    const std::string master_name;
    const std::string server_port;

    std::set<Machine> workers;      // future implementation: handle errors
    std::map<Machine, MachineStatus> workers;

    Phase phase;

    std::map<Task, TaskStatus> map_task_statuses;

    // map_task_assignments and map_machine_assignments form a 1-to-1 map
    std::map<Task, Machine> map_task_assignments;
    std::map<Machine, Task> map_machine_assignments;

    std::map<Task, TaskStatus> reduce_task_statuses;

    // reduce_task_assignments and reduce_machine_assignments form a 1-to-1 map
    std::map<Task, Machine> reduce_task_assignments;
    std::map<Machine, Task> reduce_machine_assignments;


    /**
     * Assigns a task (either map or reduce) to a worker and updates this Master's member data
     * structures accordingly. Returns the Task (std::string) assigned. If no task found, the
     * returned Task (string) is empty.
     */
    Task assign_task(const Machine &client);

    /**
     * Sends the given message (msg) to the client specified by client_fd, ensuring that all bytes
     * are sent. Returns true if no errors occurred, false otherwise.
     */
    bool send_to_client(int client_fd, std::string msg);
public:
    Master(std::string master_name,
            std::string server_port,
            std::vector<std::string> input_files,
            int num_splits);

    int start_server();
};

#endif
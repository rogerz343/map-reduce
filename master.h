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

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "definitions.h"
#include "machine.h"

class Master {
private:
    static const int QUEUE_LENGTH = 10;
    static const int BUFFER_SIZE = 4096;
    std::vector<char> buffer;

    const std::string master_name;
    const std::string server_port;
    std::set<Machine> workers;

    Phase phase;

    std::map<Task, TaskStatus> map_task_statuses;
    std::map<Task, Machine> map_task_assignments;       // future implementation: handle failures

    std::map<Task, TaskStatus> reduce_task_statuses;
    std::map<Task, Machine> reduce_task_assignments;    // future implementation: handle failures
public:
    Master(std::string master_name,
            std::string server_port,
            std::vector<std::string> input_files,
            int num_splits);

    int start_server();
};

#endif
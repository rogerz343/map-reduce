#ifndef MASTER_H
#define MASTER_H

#include <cerrno>
#include <cstring>
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
#include "maptask.h"

class Master {
private:
    static const int QUEUE_LENGTH = 10;
    static const int BUFFER_SIZE = 4096;
    std::vector<char> buffer;

    const std::string server_port;
    std::set<Machine> workers;

    Phase phase;

    std::map<MapTask, TaskStatus> map_task_status;
    std::map<MapTask, Machine> map_task_assignments;

    std::map<ReduceTask, TaskStatus> reduce_task_status;
    std::map<ReduceTask, Machine> reduce_task_assignments;
public:
    Master(std::string server_port);

    int Master::start_server();
};

#endif
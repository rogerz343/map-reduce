#ifndef MASTER_H
#define MASTER_H

#include <cerrno>
#include <cstring>
#include <iostream>
#include <set>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class Master {
private:
    static const int QUEUE_LENGTH = 10;
    static const int BUFFER_SIZE = 4096;
    std::vector<char> buffer(BUFFER_SIZE);

    const std::string server_port;
    std::set<Machine> workers;
public:
    int start_server(std::string server_port);
};

#endif
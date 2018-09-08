#include <cerrno>
#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "definitions.h"

class Worker {
private:
    const std::string worker_name;
    const std::string server_ip;
    const std::string server_port;
    const std::string map_exec;
    const std::string reduce_exec;
    const int wait_time;

    bool send_to_server(int sockfd, const std::string &msg);
    
public:
    Worker(std::string worker_name, std::string server_ip, std::string server_port,
            std::string map_exec, std::string reduce_exec,
            int wait_time);
    int run_client();
};
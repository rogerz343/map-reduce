#ifndef MACHINE_H
#define MACHINE_H

#include <string>

#include "definitions.h"

class Machine {
public:
    const std::string hostname;
    const std::string port;
    MachineStatus status;
    TaskType tasktype;

    Machine(std::string hostname, std::string port);
};

#endif
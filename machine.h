#ifndef MACHINE_H
#define MACHINE_H

#include <string>

#include "definitions.h"

class Machine {
private:
    std::string hostname;
    std::string ip;
    MachineStatus status;
    TaskType tasktype;
public:

};

#endif
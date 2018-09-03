#ifndef MACHINE_H
#define MACHINE_H

#include <string>

#include "definitions.h"

class Machine {
public:
    std::string hostname;
    std::string port;

    Machine();
    Machine(std::string hostname, std::string port);
    Machine(const Machine &right);
    Machine operator=(const Machine &right);
    bool operator<(const Machine &right) const;
    bool operator==(const Machine &right) const;
    bool operator!=(const Machine &right) const;
};

#endif
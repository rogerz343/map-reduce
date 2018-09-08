#include "machine.h"

Machine::Machine() {}

Machine::Machine(std::string hostname, std::string port) :
hostname(hostname), port(port) {}

Machine::Machine(const Machine &right) {
    hostname = right.hostname;
    port = right.port;
}

Machine Machine::operator=(const Machine &right) {
    hostname = right.hostname;
    port = right.port;
    return *this;
}

bool Machine::operator<(const Machine &right) const {
    if (hostname == right.hostname) { return port < right.port; }
    return hostname < right.hostname;
}

bool Machine::operator==(const Machine &right) const {
    return hostname == right.hostname && port == right.port;
}

bool Machine::operator!=(const Machine &right) const {
    return !(*this == right);
}
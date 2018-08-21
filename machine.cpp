#include "machine.h"

Machine::Machine(std::string hostname, std::string port) :
hostname(hostname), port(port), status(MachineStatus::available), tasktype(TaskType::none) {}
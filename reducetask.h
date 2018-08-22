#ifndef REDUCETASK_H
#define REDUCETASK_H

#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "definitions.h"

class ReduceTask {
private:
    std::unordered_map<std::string, std::vector<std::string>> inputs;
    std::unordered_map<std::string, std::vector<std::string>> outputs;

    std::function<std::vector<std::string>(std::pair<std::string, std::vector<std::string>>)> reduce_func;

public:
    ReduceTask(std::unordered_map<std::string, std::vector<std::string>> inputs,
            std::unordered_map<std::string, std::vector<std::string>> outputs,
            std::function<std::vector<std::string>(std::pair<std::string, std::vector<std::string>>)> reduce_func);
    
    void run_task();

    friend std::istream& operator>>(std::istream &is, ReduceTask &rt);
    friend std::ostream& operator<<(std::ostream &os, const ReduceTask &rt);
};

#endif
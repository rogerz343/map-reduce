#ifndef MAPTASK_H
#define MAPTASK_H

#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "definitions.h"

class MapTask {
private:
    std::vector<std::pair<std::string, std::string>> inputs;
    std::unordered_map<std::string, std::vector<std::string>> outputs;

    std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func;

public:
    MapTask(std::vector<std::pair<std::string, std::string>> inputs,
            std::unordered_map<std::string, std::vector<std::string>> outputs,
            std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func);
    
    void run_task();

    friend std::istream& operator>>(std::istream &is, MapTask &mt);
    friend std::ostream& operator<<(std::ostream &os, const MapTask &mt);
};

#endif
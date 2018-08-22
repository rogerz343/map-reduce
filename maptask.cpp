#include "maptask.h"

MapTask::MapTask(std::vector<std::pair<std::string, std::string>> inputs,
        std::unordered_map<std::string, std::vector<std::string>> outputs,
        std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func) :
        inputs(inputs), outputs(outputs), map_func(map_func) {}

void MapTask::run_task() {
    for (std::pair<std::string, std::string> &input : inputs) {
        std::vector<std::pair<std::string, std::string>> output = map_func(input);
        for (std::pair<std::string, std::string> &kv : output) {
            if (outputs.find(kv.first) == outputs.end()) {
                outputs[kv.first] = std::vector<std::string>();
            }
            outputs[kv.first].push_back(kv.second);
        }
    }
}

std::istream& operator>>(std::istream &is, MapTask &mt) {

}

std::ostream& operator<<(std::ostream &os, const MapTask &mt) {

}
#include "reducetask.h"

ReduceTask::ReduceTask(std::unordered_map<std::string, std::vector<std::string>> inputs,
        std::unordered_map<std::string, std::vector<std::string>> outputs,
        std::function<std::vector<std::string>(std::pair<std::string, std::vector<std::string>>)> reduce_func) :
        inputs(inputs), outputs(outputs), reduce_func(reduce_func) {}

void ReduceTask::run_task() {
    for (auto &input : inputs) {
        std::vector<std::string> output = reduce_func(input);
        std::string key = input.first;
        outputs[key] = output;
    }
}

std::istream& operator>>(std::istream &is, ReduceTask &rt) {

}

std::ostream& operator<<(std::ostream &os, const ReduceTask &rt) {

}
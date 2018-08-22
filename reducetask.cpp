#include "reducetask.h"

ReduceTask::ReduceTask(std::string taskname,
        std::function<std::string(std::pair<std::string, std::vector<std::string>>)> reduce_func) :
        taskname(taskname), reduce_func(reduce_func) {}

void ReduceTask::run_task() {
    std::ifstream input_file(taskname + ".rtin");
    if (!input_file.is_open()) {
        // error; stop
        return;
    }
    std::ofstream output_file(taskname + ".rtout", std::ios::trunc);
    if (!output_file.is_open()) {
        // error; stop
        return;
    }

    std::string key_line;
    std::string val_line;
    while (std::getline(input_file, key_line) && std::getline(input_file, val_line)) {
        std::vector<std::string> values;
        int index = 0;
        std::string token;
        while ((index = val_line.find(DELIMITER)) != std::string::npos) {
            values.push_back(val_line.substr(0, index));
            val_line.erase(0, index + 1);
        }
        std::string output = reduce_func(std::pair<std::string, std::vector<std::string>>(key_line, values));
        output_file << key_line << DELIMITER << output << std::endl;
    }

    input_file.close();
    output_file.close();
}

std::istream& operator>>(std::istream &is, ReduceTask &rt) {

}

std::ostream& operator<<(std::ostream &os, const ReduceTask &rt) {

}
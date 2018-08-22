#include "maptask.h"

MapTask::MapTask(std::string taskname,
        std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func) :
        taskname(taskname), map_func(map_func) {}

void MapTask::run_task() {
    std::ifstream input_file(taskname + ".mtin");
    if (!input_file.is_open()) {
        // error; stop
        return;
    }
    std::ofstream output_file(taskname + ".mtout", std::ios::trunc);
    if (!output_file.is_open()) {
        // error; stop
        return;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        int delim_idx = line.find(DELIMITER);
        std::pair<std::string, std::string> kv_in(line.substr(0, delim_idx), line.substr(delim_idx + 1));
        std::vector<std::pair<std::string, std::string>> output = map_func(kv_in);
        for (std::pair<std::string, std::string> &kv_out : output) {
            output_file << kv_out.first << DELIMITER << kv_out.second << std::endl;
        }
    }

    input_file.close();
    output_file.close();
}

std::istream& operator>>(std::istream &is, MapTask &mt) {

}

std::ostream& operator<<(std::ostream &os, const MapTask &mt) {

}
/*
 * reducetask_template.cpp
 * Template code for the executable that will run on worker machines. Users of
 * the program should make a copy of this source file, implement the reduce_func
 * function, and compile to create an executable. The name of this executable
 * should be "reducetask.cpp".
 * 
 * The input data is read from the file "[taskname].rtin" (where [taskname] is
 * the name of the task, and this file is assumed to exist). The input file
 * represents a map<string, vector<string>>. Each map consists of two lines. The
 * first line contains the key. The second line contains the value, where each
 * string in the vector<string> is delimited by DELIMITER_INLINE and each line
 * is delimited by DELIMITER_NEWLINE (as opposed to '\n'); these constants are
 * defined in "definitions.h".
 * 
 * The output data is stored in the file "[taskname].rtout". Each line of the
 * output file is a key-value pair, with each pair delimited by DELIMITER_INLINE
 * and each line delimited by DELIMITER_NEWLINE.
 */

#include <fstream>
#include <iostream>
#include <vector>

#include "definitions.h"

// ATTENTION: rewrite this function to do the desired task
std::string reduce_func(std::pair<std::string, std::vector<std::string>> input) {
    return std::to_string(input.second.size());
}

int run_task(std::string taskname) {
    std::ifstream input_file(taskname + ".rtin");
    if (!input_file.is_open()) { return 1; }
    std::ofstream output_file(taskname + ".rtout", std::ios::trunc);
    if (!output_file.is_open()) { return 1; }

    std::string key_line;
    std::string val_line;
    while (std::getline(input_file, key_line) && std::getline(input_file, val_line)) {
        std::vector<std::string> values;
        size_t index = 0;
        std::string token;
        while ((index = val_line.find(DELIMITER_INLINE)) != std::string::npos) {
            values.push_back(val_line.substr(0, index));
            val_line.erase(0, index + 1);
        }
        std::string output = reduce_func(std::make_pair(key_line, values));
        output_file << key_line << DELIMITER_INLINE << output << DELIMITER_NEWLINE;
    }

    input_file.close();
    output_file.close();
    return 0;
}

/**
 * First command line argument is the name of this executable.
 * Second command line argument is the name of the task.
 * Returns 0 on success, 1 on failure.
 */
int main(int argc, char *argv[]) {
    std::string taskname = argv[1];
    return run_task(taskname);
}
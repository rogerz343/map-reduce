/**
 * maptask_template.cpp
 * Template code for the executable that will run on worker machines. Users of
 * the program should make a copy of this source file, implement the map_func
 * function, and compile to create an executable. The name of this executable
 * should be "maptask".
 * 
 * The input data is read from the file "[taskname].mtin" (where [taskname] is
 * the name of the task, and this file is assumed to exist). Each line of the
 * input file should be a key-value pair, with each pair delimited by
 * DELIMITER_INLINE and each line delimited by DELIMITER_NEWLINE (as opposed to
 * '\n'); these constants are defined in "definitions.h".
 * 
 * The output data is stored in the file "[taskname].mtout". Each line of the
 * output file is a key-value pair, with each pair delimited by DELIMITER_INLINE
 * and each line delimited by DELIMITER_NEWLINE.
 */

#include <fstream>
#include <iostream>
#include <vector>

#include "definitions.h"

// ATTENTION: rewrite this function for your intended task.
std::vector<std::pair<std::string, std::string>> map_func(std::pair<std::string, std::string> input) {
    std::vector<std::pair<std::string, std::string>> res;
    std::string &text = input.second;
    std::string token;
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        if ((c >= 'a' && c <= 'z')
                || (c >= 'A' && c <= 'Z')
                || c == '-') {
            token += c;
        } else {
            if (!token.empty()) {
                res.push_back(std::make_pair(token, "1"));
                token.clear();
            }
        }
    }
    if (!token.empty()) { res.push_back(std::make_pair(token, "1")); }
    return res;
}

// returns 0 on success, 1 on failure
int run_task(std::string taskname) {
    std::ifstream input_file(taskname + ".mtin");
    if (!input_file.is_open()) { return 1; }
    std::ofstream output_file(taskname + ".mtout", std::ios::trunc);
    if (!output_file.is_open()) { return 1; }

    std::string line;
    while (std::getline(input_file, line, DELIMITER_NEWLINE)) {
        int delim_idx = line.find(DELIMITER_INLINE);
        std::pair<std::string, std::string> kv_in(line.substr(0, delim_idx), line.substr(delim_idx + 1));
        std::vector<std::pair<std::string, std::string>> output = map_func(kv_in);
        for (std::pair<std::string, std::string> &kv_out : output) {
            output_file << kv_out.first << DELIMITER_INLINE << kv_out.second << DELIMITER_NEWLINE;
        }
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
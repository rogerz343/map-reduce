/*
 * reducetask_template.cpp
 * Template code for the executable that will run on worker machines. Users of
 * the program should make a copy of this source file, implement the reduce_func
 * function, and compile to create an executable. The name of this executable
 * should be "reducetask.cpp".
 * 
 * The input file is formatted as follows: the name of the file is the key. The
 * file contains the name of a file on each line. The contents of each of these
 * files is a (key, value) pair where the key and value are separated by a
 * DELIMITER_NEWLINE (see definitions.h).
 * 
 * The output file is saved in red_out (see definitions.h). The filename is the
 * same as the input filename (which is the key) but with the prefix "red_out_".
 * The contents of the file is exactly the string that reduce_func produces
 * (up to the user to decide formatting).
 */

#include <fstream>
#include <iostream>
#include <vector>

#include "definitions.h"

// ATTENTION: rewrite this function to do the desired task
std::string reduce_func(std::pair<std::string, std::vector<std::string>> &input) {
    return std::to_string(input.second.size());
}

/* returns file name without its directory path */
std::string basename(const std::string &filepath) {
    size_t last_slash = filepath.find_last_of('/');
    if (last_slash != std::string::npos) {
        return filepath.substr(last_slash + 1);
    }
    return filepath;
}

int run_task(std::string kvs_file) {
    std::ifstream input_file(kvs_file);
    if (!input_file.is_open()) { return 1; }

    std::string filename = basename(kvs_file);
    std::string &red_func_key = filename;
    std::vector<std::string> red_func_value;

    std::string line;
    while (std::getline(input_file, line)) {
        std::ifstream kv_file(line);
        if (!kv_file.is_open()) { return 1; }

        std::string value;
        std::getline(kv_file, value);   // ignore the first line (the key, which we already know)
        std::getline(kv_file, value);
        red_func_value.push_back(value);    // copy by value slow; please change this later
        kv_file.close();
    }
    input_file.close();

    std::pair<std::string, std::vector<std::string>> red_func_in(red_func_key, red_func_value);
    std::string result = reduce_func(red_func_in);

    std::ofstream output_file("red_out_" + filename, std::ios::trunc);
    output_file << result;
    output_file.close();

    return 0;
}

/**
 * First command line argument is the name of the file to work on.
 * Returns 0 on success, 1 on failure.
 */
int main(int argc, char *argv[]) {
    std::string kvs_file = argv[1];
    return run_task(kvs_file);
}
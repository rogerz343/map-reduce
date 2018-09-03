/**
 * maptask_template.cpp
 * Template code for the executable that will run on worker machines. Users of
 * the program should make a copy of this source file, implement the map_func
 * function, and compile to create an executable. The name of this executable
 * should be "maptask".
 * 
 * The input file is a key value pair. The key is the name of the file that is
 * given as an argument to this executable, and the value is the contents of
 * that file.
 * 
 * The output file is a key value pair. The name of the file is relatively
 * arbitrary (but guaranteed to be unique). The key is the first line of the
 * file and the value is the rest of the contents. These lines are delimited by
 * DELIMITER_NEWLINE (as opposed to \n).
 */

#include <fstream>
#include <iostream>
#include <vector>

#include "definitions.h"

// ATTENTION: rewrite this function for your intended task.
std::vector<std::pair<std::string, std::string>> map_func(std::pair<std::string, std::string> &input) {
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

/* returns file name without its directory path */
std::string basename(const std::string &filepath) {
    size_t last_slash = filepath.find_last_of('/');
    if (last_slash != std::string::npos) {
        return filepath.substr(last_slash + 1);
    }
    return filepath;
}

// returns 0 on success, 1 on failure
int run_task(std::string kv_file) {
    std::ifstream input_file(kv_file);
    if (!input_file.is_open()) { return 1; }

    std::string filename = basename(kv_file);
    std::string file_text;
    file_text.assign(std::istreambuf_iterator<char>(input_file),
                     std::istreambuf_iterator<char>());

    std::pair<std::string, std::string> kv_in = std::make_pair(filename, file_text);
    std::vector<std::pair<std::string, std::string>> output = map_func(kv_in);
    int id = 0;     // used to make output filenames unique
    for (std::pair<std::string, std::string> &kv_out : output) {
        std::ofstream output_file(map_out + filename + "_" + std::to_string(id), std::ios::trunc);
        if (!output_file.is_open()) { return 1; }

        output_file << kv_out.first;
        output_file << DELIMITER_NEWLINE;
        output_file << kv_out.second;
        output_file.close();
        ++id;
    }

    input_file.close();
    return 0;
}

/**
 * 1st command line argument is the name of the file to work on.
 * Returns 0 on success, 1 on failure.
 */
int main(int argc, char* argv[]) {
    std::string kv_file(argv[1]);
    return run_task(kv_file);
}
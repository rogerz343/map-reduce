/**
 * reducetask.h
 * Defines the ReduceTask class, which represents a "job" to run on a worker
 * machine.
 * 
 * The input data is read from the file "[taskname].rtin" (where [taskname] is
 * the name of the task, and this file is assumed to exist). The input file
 * represents a map<string, vector<string>>. Each map consists of two lines. The
 * first line contains the key. The second line contains the value, where each
 * string in the vector<string> is delimited by DELIMITER (defined in
 * "definitions.h").
 * 
 * The output data is stored in the file "[taskname].rtout". Each line of the
 * output file is a key-value pair, with each pair delimited by DELIMITER.
 */

#ifndef REDUCETASK_H
#define REDUCETASK_H

#include <functional>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "definitions.h"

class ReduceTask {
private:
    const std::string taskname;
    const std::function<std::string(std::pair<std::string, std::vector<std::string>>)> reduce_func;

public:
    ReduceTask(std::string taskname,
            std::function<std::string(std::pair<std::string, std::vector<std::string>>)> reduce_func);
    
    void run_task();

    friend std::istream& operator>>(std::istream &is, ReduceTask &rt);
    friend std::ostream& operator<<(std::ostream &os, const ReduceTask &rt);
};

#endif
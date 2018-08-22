/**
 * maptask.h
 * Defines the MapTask class, which represents a "job" to run on a worker machine.
 * 
 * The input data is read from the file "[taskname].mtin" (where [taskname] is
 * the name of the task, and this file is assumed to exist). Each line of the
 * input file should be a key-value pair, with each pair delimited by DELIMITER
 * (defined in "definitions.h").
 * 
 * The output data is stored in the file "[taskname].mtout". Each line of the
 * output file is a key-value pair, with each pair delimited by DELIMITER.
 */

#ifndef MAPTASK_H
#define MAPTASK_H

#include <functional>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "definitions.h"

class MapTask {
private:
    const std::string taskname;
    const std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func;

public:
    MapTask(std::string taskname,
            std::function<std::vector<std::pair<std::string, std::string>>(std::pair<std::string, std::string>)> map_func);
    
    void run_task();

    friend std::istream& operator>>(std::istream &is, MapTask &mt);
    friend std::ostream& operator<<(std::ostream &os, const MapTask &mt);
};

#endif
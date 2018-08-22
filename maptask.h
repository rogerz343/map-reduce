#ifndef MAPTASK_H
#define MAPTASK_H

#include <functional>
#include <iostream>
#include <unordered_map>
#include <vector>

#include "definitions.h"

template <typename Kin, typename Vin, typename Kout, typename Vout>
class MapTask {
private:
    std::vector<std::pair<Kin, Vin>> inputs;
    std::unordered_multimap<Kout, std::vector<Vout>> outputs;
public:
    std::function<std::pair<Kout, Vout>(std::pair<Kin, Vin>)> map_func;

    MapTask(std::vector<std::pair<Kin, Vin>> inputs,
            std::vector<std::pair<Kout, Vout>> outputs,
            std::function<std::pair<Kout, Vout>(std::pair<Kin, Vin>)> map_func) :
            inputs(inputs), outputs(outputs), map_func(map_func) {}
    
    void run_task() {
        while (!inputs.empty()) {
            std::vector<std::pair<Kin, Vin>> input = inputs.pop_back();
            outputs.insert(map_func(input));
        }
    }

    template <typename Ki, typename Vi, typename Ko, typename Vo>
    friend std::istream& operator>>(std::istream &is, MapTask<Ki, Vi, Ko, Vo> &mt);

    template <typename Ki, typename Vi, typename Ko, typename Vo>
    friend std::ostream& operator<<(std::ostream &os, const MapTask<Kin, Vin, Kout, Vout> &mt);
};

#endif
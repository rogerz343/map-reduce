#ifndef JOB_H
#define JOB_H

#include <set>

class Setup {
private:
    Machine master;
    std::set<Machine> workers;
public:
    Setup();
    ~Setup();

    void add_master
    void add_worker(std::string user, std::string hostname, std::string port);
};

#endif;
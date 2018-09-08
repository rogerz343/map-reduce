#include "master.h"
#include <dirent.h>

/**
 * Runs the master for the map/reduce job.
 * 
 * arg1: name for the job (not really important)
 * arg2: directory to the input files.
 * arg3: port to run on
 * arg4: number of partitions to split input files into
 */
int main(int argc, char *argv[]) {
    if (argc < 5) {
        std::cout << "Usage: ./run_master [name] [path-to-input] [port] [number_of_input partitions]" << std::endl;
        return 1;
    }
    std::string input_dir(argv[2]);
    std::vector<std::string> input;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir(input_dir.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string filename(ent->d_name);
            if (filename == "." || filename == "..") { continue; }
            input.push_back(input_dir.c_str() + std::string(ent->d_name));
        }
        closedir(dir);
    } else {
        std::cout << "hmMMmMmmMmMMMMM" << std::endl;
        // perror ("");
        return 1;
    }
    Master m(argv[1], argv[3], input, 1);
    m.start_server();
}
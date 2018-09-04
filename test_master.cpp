#include "master.h"

#include <dirent.h>

int main() {
    std::vector<std::string> input;
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir("./ex_word_count_in")) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            std::string filename(ent->d_name);
            if (filename == "." || filename == "..") { continue; }
            input.push_back("./ex_word_count_in/" + std::string(ent->d_name));
        }
        closedir(dir);
        return 1;
    } else {
        std::cout << "hmMMmMmmMmMMMMM" << std::endl;
        // perror ("");
        return 1;
    }
    Master m("wordcount", "8000", input, 1);
    m.start_server();
}
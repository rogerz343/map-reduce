#include "master.h"

int main() {
    std::vector<std::string> input = {
        "./ex_word_count_in/bird.txt",
        "./ex_word_count_in/cow.txt",
        "./ex_word_count_in/penguin.txt"
    };
    Master m("wordcount", "8000", input, 1);
    m.start_server();
}
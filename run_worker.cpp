#include "worker.h"

/**
 * arg1: a unique name for this worker
 * arg2: the ip address of the server
 * arg3: the port of the server
 * arg4: the filepath to the maptask executable
 * arg5: the filepath to the reducetask executable
 * arg6 (optional): time to idle when server tells this worker to wait
 */
int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cout << "Usage: ./run_worker [worker_name] [server ip] [server port] [maptask filepath] [reducetask filepath] [wait_time]" << std::endl;
        return 1;
    }
    Worker worker(argv[1], argv[2], argv[3], argv[4], argv[5], atoi(argv[6]));
    return worker.run_client();
}
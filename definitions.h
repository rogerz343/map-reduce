#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Some definitions that are used throughout the program. They should
// probably be put in more specific .h files.

// A Task is defined as the input filepath. This path is relative to the root
// directory of this project.
typedef std::string Task;

// location of each split of the map function inputs
constexpr char map_in_splits[] = "./map_in_splits/";
constexpr char map_out[] = "./map_out/";
constexpr char key_groups[] = "./intermediate_out/";
constexpr char red_in_splits[] = "./red_in_splits/";
constexpr char red_out[] = "./red_out/";


enum class TaskType { none, map_task, reduce_task };
enum class TaskStatus { unassigned, in_progress, finished };
enum class MachineStatus { idle, in_progress, finished, failed };
enum class Phase { map_phase, intermediate_phase, reduce_phase, finished_phase };

// message that client sends to server when it connects for the first time
constexpr char CONNECT_MSG[] = "###connected###";
constexpr int CONNECT_MSG_LEN = 15;

// message that server sends to client to indicate a new task
// ***NOTE*** this is currently unused
constexpr char NEW_TASK_MSG[] = "###new_task_file###";
constexpr int NEW_TASK_MSG_LEN = 19;

// message that client sends to server when it finishes a task
constexpr char FIN_TASK_MSG[] = "###finished_task###";
constexpr int FIN_TASK_MSG_LEN = 19;

// message that server sends to client to tell it to wait and reconnect later
constexpr char WAIT_MSG[] = "###wait###";
constexpr int WAIT_MSG_LEN = 10;

// message that server sends to client to tell it that everything is done and
// it's safe to disconnect.
constexpr char DISCONNECT_MSG[] = "###disconnect###";
constexpr int DISCONNECT_MSG_LEN = 16;

// characters that are hopefully not in actual data
constexpr char DELIMITER_NEWLINE = '|'; // '\037';
constexpr char DELIMITER_INLINE = '^';

#endif
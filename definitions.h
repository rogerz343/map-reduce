#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Some definitions that are used throughout the program. They should
// probably be put in more specific .h files.

// A Task is defined as the input filename.
typedef std::string Task;

// location of each split of the map function inputs
constexpr char map_in_splits[] = "./map_in_splits/";
constexpr char map_out[] = "./map_out/";
constexpr char red_in_splits[] = "./red_in_splits/";
constexpr char red_out_splits[] = "./red_out_splits/";


enum class TaskType { none, map_task, reduce_task };
enum class TaskStatus { unassigned, in_progress, finished };
enum class MachineStatus { available, in_progress, finished, failed };
enum class Phase { map_phase, intermediate_phase, reduce_phase, finished_phase };

constexpr char CONNECT_MSG[] = "###connected###";
constexpr int CONNECT_MSG_LEN = 15;

constexpr char NEW_TASK_MSG[] = "###new_task_file###";
constexpr int NEW_TASK_MSG_LEN = 19;

constexpr char FIN_TASK_MSG[] = "###finished_task###";
constexpr int FIN_TASK_MSG_LEN = 19;

constexpr char DELIMITER_NEWLINE = '\037';
constexpr char DELIMITER_INLINE = '^';

#endif
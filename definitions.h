#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Some definitions that are used throughout the program. They should
// probably be put in more specific .h files.

// A Task is defined as the input filename.
typedef std::string Task;

enum class TaskType { none, map_task, reduce_task };
enum class TaskStatus { unassigned, in_progress, finished };
enum class MachineStatus { available, in_progress, finished, failed };
enum class Phase { map_phase, intermediate_phase, reduce_phase, finished_phase };

constexpr char CONNECT_MSG[] = "connected";
constexpr char CONNECT_MSG_LEN = 9;

constexpr char NEW_TASK_MSG[] = "new_task";
constexpr char NEW_TASK_MSG_LEN = 8;

constexpr char DELIMITER_NEWLINE = '\037';
constexpr char DELIMITER_INLINE = '^';

#endif
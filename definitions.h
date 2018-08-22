#ifndef DEFINITIONS_H
#define DEFINITIONS_H

// Some definitions that are used throughout the program. They should
// probably be put in more specific .h files.

enum class TaskType { none, map_task, reduce_task };
enum class TaskStatus { unassigned, in_progress, finished };
enum class MachineStatus { available, in_progress, finished, failed };
enum class Phase { map_phase, reduce_phase };

constexpr char connect_msg[] = "connected";
constexpr char connect_msg_len = 9;

#endif
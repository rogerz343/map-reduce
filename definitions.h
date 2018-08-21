#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef void (*TaskFunction)(void);

enum class TaskType { none, map_task, reduce_task };
enum class MachineStatus { available, in_progress, finished, failed };
enum class Phase { map_phase, reduce_phase };

constexpr char connect_msg[] = "connected";
constexpr char connect_msg_len = 9;

#endif
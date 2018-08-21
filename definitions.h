#ifndef DEFINITIONS_H
#define DEFINITIONS_H

typedef void (*TaskFunction)(void);

enum TaskType { none, map_task, reduce_task };
enum MachineStatus { available, in_progress, finished, failed };

#endif
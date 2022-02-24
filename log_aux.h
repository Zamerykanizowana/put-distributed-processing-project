#ifndef GATES_LOG_AUX_H
#define GATES_LOG_AUX_H
#include <mpi.h>
#include "log.h"
#include "tourist.h"

extern const int LOGGER_TAG;
extern const char *level_strings[];
extern const char *proc_colors[];

void send_log_Event(log_Event *ev);
void kill_everyone(log_Event *ev);
#endif

#include "log_aux.h"

const int LOGGER_TAG = 666;

const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

const char *proc_colors[] = {
	"", "\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m",
	"\x1b[31;1m", "\x1b[32;1m", "\x1b[33;1m", "\x1b[34;1m", "\x1b[35;1m", "\x1b[36;1m"
};

void send_log_Event(log_Event *ev) {
	char buf[16];
	buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';

	char fmt_buf[128];
	char final_buf[256];

	vsprintf(fmt_buf, ev->fmt, ev->ap);

	sprintf(
			final_buf, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m %s %s \x1b[0m",
			buf, level_colors[ev->level], level_strings[ev->level],
			ev->file, ev->line, proc_colors[T.rank], fmt_buf);

	MPI_Send(final_buf, 
			sizeof(char)*256, 
			MPI_CHAR, 
			0, 
			LOGGER_TAG, 
			MPI_COMM_WORLD);
}

void kill_everyone(log_Event *ev) {
	send_log_Event(ev);
	MPI_Abort(MPI_COMM_WORLD, 3);
}

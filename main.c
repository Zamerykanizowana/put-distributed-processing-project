#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "log.h"
#include "tourist.h"
#include "msg.h"
#include "handlers.h"
#define ROOT 0
#define MSG_TAG 100

// BEGIN LOGGING-RELATED STRUCTS/FUNCS
// TODO: move to external file
const int LOGGER_TAG = 666;

static const char *level_strings[] = {
  "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

static const char *level_colors[] = {
  "\x1b[94m", "\x1b[36m", "\x1b[32m", "\x1b[33m", "\x1b[31m", "\x1b[35m"
};

static const char *proc_colors[] = {
	"", "\x1b[31m", "\x1b[32m", "\x1b[33m", "\x1b[34m", "\x1b[35m", "\x1b[36m",
	"\x1b[31;1m", "\x1b[32;1m", "\x1b[33;1m", "\x1b[34;1m", "\x1b[35;1m", "\x1b[36;1m"
};

static void send_log_Event(log_Event *ev) {
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

static void kill_everyone(log_Event *ev) {
	send_log_Event(ev);
	MPI_Abort(MPI_COMM_WORLD, 3);
}
// END LOGGING-RELATED STRUCTS/FUNCS

void sigint_handler(int num) {
	log_info("%d is closing...", T.rank);

	MPI_Finalize();
}

void logger_sink_event_loop() {
	while (1) {
		char msg[256];
		MPI_Status status;

		MPI_Recv(msg, sizeof(char)*256, MPI_CHAR, MPI_ANY_SOURCE, 
				LOGGER_TAG, MPI_COMM_WORLD, &status);

		printf("\x1b[7m%s[%d]\x1b[0m %s\n", proc_colors[status.MPI_SOURCE], status.MPI_SOURCE, msg);
	}
}

void main_event_loop() {
	int i = 0;

	enter_store();

	while (1) {
		log_info("Loopin': %d, responses: %d", i, T.responses);

		general_msg incoming_msg;
		MPI_Status status;

		log_info("State: %d", T.state);
		log_info("I think that %d/%d store slots are free", 
				T.free_store_slots, T.total_store_slots);
		T_print_res();	

		// Blocking receive.
		MPI_Recv(&incoming_msg,
				sizeof(general_msg),
				MPI_BYTE,
				MPI_ANY_SOURCE,
				MPI_ANY_TAG,
				MPI_COMM_WORLD,
				&status
			);

		T.clk++;

		log_info("Message received from %d", status.MPI_SOURCE);

		switch (status.MPI_TAG) {
			case REQ_STORE:
				log_info("REQ_STORE received!");
				handle_req_store(status.MPI_SOURCE, incoming_msg);
				break;
			case REL_STORE:
				log_info("REL_STORE received!");
				handle_release_store(status.MPI_SOURCE);
				break;
			case ACK:
				log_info("ACK received!");
				handle_ack(status.MPI_SOURCE, incoming_msg);
				break;
			default:
				log_error("Unknown message tag %d", status.MPI_TAG);
				break;
		}

		switch (T.state) {
			case REQUESTING_STORE_SLOT:
				handle_waiting_for_store_state();
				break;
			case WAITING_FOR_FREE_STORE_SLOT:
				handle_waiting_for_free_store_slot();
				break;
			case OCCUPYING_STORE_SLOT:
				break;
			default:
				log_error("Unknown state %d", T.state);
				break;
		}

		i++;
	}
}

int main(int argc, char **argv) {
	// Register signal handler.
	signal(SIGINT, sigint_handler);

	MPI_Init(&argc, &argv);

	// Gather information about process count and pool size.
	MPI_Comm_size(MPI_COMM_WORLD, &T.size);
	MPI_Comm_rank(MPI_COMM_WORLD, &T.rank);

	// This is just a test function to see if a global variable
	// that was declared in other file and modified by code in this file
	// behaves properly (i.e. the modification has propagated).
	print_size_rank();

	// Set initial state.
	T.state = REQUESTING_STORE_SLOT; 

	// Initialize resources list for storing local bits of information
	// about other processes.
	T.res = (world_resources *) malloc(T.size * sizeof(world_resources));
	for (int i = 1; i < T.size; i++) {
		world_resources res = {0, 0};
		T.res[i] = res;
	}

	// Get environment variables.
	const char *total_store_slots_env = getenv("STORE_SLOTS");	

	// Set total store slots with fallback.
	if (total_store_slots_env) {
		T.total_store_slots = strtol(total_store_slots_env, NULL, 10);
	} else {
		T.total_store_slots = 2;
	}

	// Initialize free slots.
	T.free_store_slots = T.total_store_slots;

	log_add_callback(kill_everyone, NULL, LOG_ERROR);

	// The function below never exits.
	if (T.rank == 0) {
		// Not the main loop.
		logger_sink_event_loop();
	} else {
		log_set_quiet(1);
		log_add_callback(send_log_Event, NULL, LOG_TRACE);
		main_event_loop();
	}
}

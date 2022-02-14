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

static void send_log_Event(log_Event *ev) {
	char buf[16];
	buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';

	char fmt_buf[128];
	char final_buf[256];

	vsprintf(fmt_buf, ev->fmt, ev->ap);

	sprintf(
			final_buf, "%s %s%-5s\x1b[0m \x1b[90m%s:%d:\x1b[0m %s",
			buf, level_colors[ev->level], level_strings[ev->level],
			ev->file, ev->line, fmt_buf);

	MPI_Send(final_buf, 
			sizeof(char)*256, 
			MPI_CHAR, 
			0, 
			LOGGER_TAG, 
			MPI_COMM_WORLD);
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

		printf("[%d] %s\n", status.MPI_SOURCE, msg);
	}
}

void main_event_loop() {
	int i = 0;

	enter_store();

	while (1) {
		log_info("%d is loopin': %d, responses: %d", T.rank, i, T.responses);

		general_msg incoming_msg;
		MPI_Status status;

		log_info("%d will receive, might block till the end of times!",
				T.rank);

		log_info("%d state: %d", T.rank, T.state);
		log_info("%d thinks %d/%d store slots are free", 
				T.rank, T.free_store_slots, T.total_store_slots);	

		// Blocking receive.
		MPI_Recv(&incoming_msg,
				sizeof(general_msg),
				MPI_BYTE,
				MPI_ANY_SOURCE,
				MPI_ANY_TAG,
				MPI_COMM_WORLD,
				&status
			);

		log_info("%d received a message from %d", T.rank, status.MPI_SOURCE);

		switch (status.MPI_TAG) {
			case REQ_STORE:
				log_info("REQ_STORE received!");
				handle_req_store(status.MPI_SOURCE, incoming_msg);
				break;
			case ACK:
				log_info("ACK received!");
				handle_ack(status.MPI_SOURCE, incoming_msg);
				break;
			case NACK:
				log_info("NACK received!");
				handle_nack(status.MPI_SOURCE, incoming_msg);
				break;
			default:
				log_error("Unknown message tag %d", status.MPI_TAG);
				break;
		}

		switch (T.state) {
			case WAITING_FOR_STORE:
				handle_waiting_for_store_state();
				break;
			default:
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
	T.state = WAITING_FOR_STORE; 

	// Initialize resources list for storing local bits of information
	// about other processes.
	T.res = (world_resources *) malloc(T.size * sizeof(world_resources));
	for (int i = 0; i < T.size; i++) {
		world_resources res = {0};
		T.res[i] = res;
	}	

	// TODO: HARD-CODED ENVIRONMENT!!!
	T.total_store_slots = 2;
	T.free_store_slots = T.total_store_slots;

	// The function below never exits.
	if (T.rank == 0) {
		// Not the main loop.
		logger_sink_event_loop();
	} else {
		log_set_quiet(1);
		log_add_callback(send_log_Event, NULL, LOG_INFO);
		main_event_loop();
	}
}

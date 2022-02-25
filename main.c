#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>
#include "log.h"
#include "log_aux.h"
#include "tourist.h"
#include "msg.h"
#include "handlers.h"
#define ROOT 0
#define MSG_TAG 100

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

	enter_store_req();

	while (1) {
		log_info("Looping: %d, responses: %d", i, T.responses);

		general_msg incoming_msg;
		MPI_Status status;

		log_info("State: %d", T.state);
		//if (T.state == WAITING_FOR_STORE) {
			log_info("I think that %d/%d store slots are free", 
				T.free_store_slots, T.total_store_slots);
			T_print_res();	
		//}
		
		T_print_res_psychic();

		// Blocking receive.
		MPI_Recv(&incoming_msg,
				sizeof(general_msg),
				MPI_BYTE,
				MPI_ANY_SOURCE,
				MPI_ANY_TAG,
				MPI_COMM_WORLD,
				&status
			);
		
		//T.clk++;
		//log_info("Message received from %d", status.MPI_SOURCE);
		

		// Allow storing 3 characters + null termination.
		char aux_log[4] = {'\0'};

		// Differentiate between (N)ACKs for store vs. psychic.
		if (T.state == WAITING_FOR_PSYCHIC) {
			strcpy(aux_log, "_PS");
		}

		switch (status.MPI_TAG) {
			case REQ_STORE:
				log_info("REQ_STORE received from %d! Incoming clk %d, my clk %d", 
					status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_req_store(status.MPI_SOURCE, incoming_msg);
				break;
			case REL_STORE:
				log_info("REL_STORE received from %d! Incoming clk %d, my clk %d", 
					status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_release_store(status.MPI_SOURCE);
				break;
			case ACK:
				log_info("ACK%s received from %d! Incoming clk %d, my clk %d", 
					aux_log, status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_ack(status.MPI_SOURCE, incoming_msg);
				break;
			case NACK:
				log_info("NACK%s received from %d! Incoming clk %d, my clk %d", 
					aux_log, status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_nack(status.MPI_SOURCE, incoming_msg);
				break;
			case REQ_PSYCHIC:
				log_info("REQ_PSYCHIC received from %d! Incoming clk %d, my clk %d",
					status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_req_psychic(status.MPI_SOURCE, incoming_msg);
				break;
			case ENTER_TUNNEL:
				log_info("ENTER_TUNNEL received from %d! Incoming clk %d, my clk %d",
					status.MPI_SOURCE, incoming_msg.clk, T.clk);
				handle_enter(status.MPI_SOURCE);
				break;
			default:
				log_error("Unknown message tag %d", status.MPI_TAG);
				break;
		}

		switch (T.state) {
			case WAITING_FOR_STORE:
				handle_waiting_for_store_state();
				break;
			case SHOPPING:
				log_info("I'm in the state of shopping!");
				break;
			case WAITING_FOR_PSYCHIC:
				log_info("Waiting for 🌑");
				handle_waiting_for_psychic();
				break;
			case TRIPPING:
				log_info("I'm surfing through the tunnel! 🏄");
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

	// Seed random number generator.
	struct timespec nanos;
	clock_gettime(CLOCK_MONOTONIC, &nanos);
	srand(nanos.tv_nsec);

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
	// about other processes in queque for store
	T.res = (world_resources *) malloc(T.size * sizeof(world_resources));
	for (int i = 1; i < T.size; i++) {
		world_resources res = {0};
		T.res[i] = res;
	}

	// Initialize resources list for storing local bits of information
	// about other pricesses in queque for psychic
	T.res_psychic = (world_resources_psychic *) malloc(T.size * sizeof(world_resources_psychic));
	for (int i = 1; i < T.size; i++) {
		world_resources_psychic res_psychic = {-1};
		T.res_psychic[i] = res_psychic;
	}

	T.res_que = (local_queue *) malloc(T.size * sizeof(local_queue));
	for (int i = 1; i < T.size; i++) {
		local_queue res_queue = {-1};
		T.res_que[i] = res_queue;
	}

	// Get environment variables.
	const char *total_store_slots_env = getenv("STORE_SLOTS");

	// Get enviorenment variables
	const char *total_psychic_slots_env = getenv("PSYCHIC_SLOTS");	

	// Set total store slots with fallback.
	if (total_store_slots_env) {
		T.total_store_slots = strtol(total_store_slots_env, NULL, 10);
	} else {
		T.total_store_slots = 2;
	}

	// Set total psychic slots with fallback
	if (total_psychic_slots_env) {
		T.total_psychic_slots = strtol(total_psychic_slots_env, NULL, 10);
	} else {
		T.total_psychic_slots = 2;
	}

	// Initialize free slots.
	T.free_store_slots = T.total_store_slots;
	T.when_break_needed = 0;


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

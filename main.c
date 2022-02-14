#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "tourist.h"
#include "msg.h"
#include "handlers.h"
#define ROOT 0
#define MSG_TAG 100

void sigint_handler(int num) {
	log_info("%d is closing...", T.rank);

	MPI_Finalize();
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
	main_event_loop();
}

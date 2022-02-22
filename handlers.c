#include <mpi.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "msg.h"
#include "tourist.h"
#include "log.h"
#include "handlers.h"

void handle_req_store(int src, general_msg msg) {
	int tag;

	if (incoming_event_happened_before(msg.clk, src)) {
		log_info("sending ACK to %d", src);
		tag = ACK;
		T_enter_store(src);
	} else {
		log_info("sending NACK to %d", src);
		tag = NACK;
	}

	general_msg new_msg = {T.clk};
	MPI_Send(&new_msg,
			sizeof(new_msg),
			MPI_BYTE,
			src,
			tag,
			MPI_COMM_WORLD
		);
}

void handle_ack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE) {
		T.responses++;
	}
}

void handle_nack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE) {
		T.responses++;
	}

	switch (T.state) {
		case WAITING_FOR_STORE:
			T_enter_store(src);
			break;
		default:
			break;
	}
}

void handle_release_store(int tourist) {
	T_leave_store(tourist);
}

void handle_waiting_for_store_state() {
	// Process only once we've gathered responses from all nodes.
	if (T.responses == T.size - 2) {
		// Consume responses (empty the stomach).
		T.responses = 0;
		
		if (T.free_store_slots > 0) {
			T.clk++;
			T.state = SHOPPING;
			T_enter_store(T.rank);
			log_info("I'm shopping now!");

			pthread_t glue_fan;
			pthread_create(&glue_fan, NULL, do_the_shopping, NULL);
		} else {
			T.state = WAITING_FOR_STORE;
			log_info("Grr, I have to wait...");
		}
	}
}

void *do_the_shopping(void *arg) {
	int duration = 3;

	log_info("I'll spend %d seconds in the store", duration);
	sleep(duration);
	log_info("Done! 🏪✅");

	T_leave_store(T.rank);

	release_store();

	T.state = WAITING_FOR_STORE;

	return NULL;
}

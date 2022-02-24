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

	
	if (T.state == WAITING_FOR_STORE || T.state == SHOPPING) {
		if (incoming_event_happened_before(msg.clk, src)) {
			if (T.state == SHOPPING) {
				tag = NACK;
			} else {	
				tag = ACK;
				T_enter_store(src);
			}
		} else {
			tag = NACK;
		}
	} else {
		tag = ACK;
		T_enter_store(src);	
	}

	if (tag == ACK) {	
		log_info("Sending ACK to %d. My clk is %d", src, T.clk);
	} else {	
		log_info("Sending NACK to %d. My clk is %d", src, T.clk);
	}

	//T.clk++;

	general_msg new_msg = {T.clk};
	MPI_Send(&new_msg,
			sizeof(new_msg),
			MPI_BYTE,
			src,
			tag,
			MPI_COMM_WORLD
		);
}

// src is process number (incoming p_id)
void handle_ack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE) {
		T.responses++;
		T.res[src].store_claimed = 0;
	}

}

void handle_nack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE) {
		T.responses++;
	}

	//TODO for Ola - need to check, what to do with that
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
		
		if (T.free_store_slots > 0) {
			// Consume responses (empty the stomach).
			T.responses = 0;
			T.clk++;
			T.state = SHOPPING;
			T_enter_store(T.rank);
			log_info("I'm shopping now! 🏪👀");

			pthread_t glue_fan;
			pthread_create(&glue_fan, NULL, do_the_shopping, NULL);
		} else {
			//T.state = WAITING_FOR_STORE;
			log_info("Grr, I have to wait... but I have all responses");
		}
	} else {
		log_info("Grr, I have to wait, I have only %d resp", T.responses);
	}
}

void *do_the_shopping(void *arg) {
	int duration = 3;

	log_info("I'll spend %d seconds in the store", duration);
	sleep(duration);
	log_info("Shopping done! 🏪✅");

	T_leave_store(T.rank);

	release_store();

	T.state = WAITING_FOR_PSYCHIC;

	//enter_store_req();

	return NULL;
}

void handle_req_psychic(int src, general_msg msg) {
	int tag;

	if (T.state == WAITING_FOR_PSYCHIC) {
		if (incoming_event_happened_before(msg.clk, src)) {
			tag = ACK;
			log_info("Sending ACK_PSYCHIC to %d. My clk is %d", src, T.clk);
			//T_enter_psychic(src) ??
		} else {
			tag = NACK;
			log_info("Sending NACK_PSYCHIC to %d. My clk is %d", src, T.clk);
		}
	} else {
		tag = ACK;
		log_info("Sending ACK_PSYCHIC to %d. My clk is %d", src, T.clk);
		//T_enter_psychic(src) ??
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

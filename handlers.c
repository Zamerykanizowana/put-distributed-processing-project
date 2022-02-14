#include <mpi.h>
#include "msg.h"
#include "tourist.h"

void handle_req_store(int src, general_msg msg) {
	int tag;

	if (incoming_event_happened_before(msg.clk, src)) {
		tag = ACK;
		T_enter_store(src);
	} else {
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

void handle_waiting_for_store_state() {
	// Process only once we've gathered responses from all nodes.
	if (T.responses == T.size - 1) {
		// Consume responses (empty the stomach).
		T.responses = 0;
		
		if (T.free_store_slots > 0) {
			T.clk++;
			T.state = SHOPPING;
		} else {
			T.state = WAITING_FOR_STORE;
		}
	}
}

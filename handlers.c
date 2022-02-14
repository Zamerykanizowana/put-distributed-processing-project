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
	T.responses++;
}

void handle_nack(int src, general_msg msg) {
	T.responses++;
	T_enter_store(src);
}

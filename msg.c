#include <mpi.h>
#include "msg.h"
#include "tourist.h"
#include "log.h"

// Check if sender wins.
// In case of timestamp conflict, lower rank wins.
int incoming_event_happened_before(int incoming_clk, int incoming_rank) {
	return (T.clk > incoming_clk || 
			(T.clk == incoming_clk && 
			 incoming_rank < T.rank)
	       );
}

void print_size_rank() {
	log_info("id %d/%d", T.size, T.rank);
}

void send_to_all(general_msg msg, msg_tag tag) {
	for (int i = 1; i < T.size; i++) {
		// Don't send message to self.
		if (i == T.rank) {
			continue;
		}

		log_debug("Sending %d to %d", tag, i);

		MPI_Send(&msg,
				sizeof(general_msg),
				MPI_BYTE,
				i,
				tag,
				MPI_COMM_WORLD
			);
	}
}

void enter_store() {
	general_msg msg = {T.clk};
	send_to_all(msg, REQ_STORE);
}

void release_store() {
	general_msg msg = {T.clk};
	send_to_all(msg, REL_STORE); 
}

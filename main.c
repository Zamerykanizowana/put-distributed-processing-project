#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "tourist.h"
#include "msg.h"
#define ROOT 0
#define MSG_TAG 100

void sigint_handler(int num) {
	log_info("%d is closing...", T.rank);

	MPI_Finalize();
}

void main_event_loop() {
	// TODO: send initial message.
	int i = 0;

	while (1) {
		log_info("%d is loopin': %d", T.rank, i);

		while (1) {
			general_msg incoming_msg;
			MPI_Status status;

			// Blocking receive.
			MPI_Recv(&incoming_msg,
					sizeof(general_msg),
					MPI_BYTE,
					MPI_ANY_SOURCE,
					MPI_ANY_TAG,
					MPI_COMM_WORLD,
					&status
				);

			switch (status.MPI_TAG) {
				case REQ_STORE:
					break;
				case ACK:
					break;
				case NACK:
					break;
				default:
					log_error("Unknown message tag %d", status.MPI_TAG);
					break;
			}
		}

		i++;
	}
}

int main(int argc, char **argv) {
	signal(SIGINT, sigint_handler);

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &T.size);
	MPI_Comm_rank(MPI_COMM_WORLD, &T.rank);

	log_info("id %d/%d", T.size, T.rank);

	main_event_loop();
}

#include <mpi.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "log.h"
#include "tourist.h"
#define ROOT 0
#define MSG_TAG 100

void sigint_handler(int num) {
	log_info("%d is closing...", T.rank);

	MPI_Finalize();
}

void main_event_loop() {
	int i = 0;

	while (1) {
		log_info("%d is loopin' and sleepin': %d", T.rank, i);
		sleep(1);
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

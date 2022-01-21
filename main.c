#include <mpi.h>
#include <stdlib.h>
#include <stdio.h>
#include "log.h"
#define ROOT 0
#define MSG_TAG 100

int main(int argc, char **argv) {
	int tid, size;
	MPI_Status status;

	MPI_Init(&argc, &argv);

	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Comm_rank(MPI_COMM_WORLD, &tid);

	log_info("id %d/%d", tid, size);

	MPI_Finalize();
}

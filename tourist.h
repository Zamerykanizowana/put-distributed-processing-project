#ifndef GATES_TOURIST_H
#define GATES_TOURIST_H

typedef enum {
	WAITING_FOR_SHOP,
	SHOPPING,
	WAITING_FOR_PSYCHIC,
	TRIPPING
} state_t;

struct tourist {
	// MPI-determined variables
	int rank;
	int size;
	// Lamport timestamp
	int clk;
	// A State of Tourist
	state_t state;
} T;

#endif

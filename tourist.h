#ifndef GATES_TOURIST_H
#define GATES_TOURIST_H

typedef enum {
	WAITING_FOR_SHOP,
	SHOPPING,
	WAITING_FOR_PSYCHIC,
	TRIPPING
} tourist_state;

typedef struct {
	int store_claimed;
} world_resources;

typedef struct tourist {
	// MPI-determined variables
	int rank;
	int size;
	// Lamport timestamp
	int clk;
	// A State of Tourist
	tourist_state state;
	// Number of responses received from other nodes
	int responses;
	world_resources *res;
} tourist;

extern tourist T;

#endif

#ifndef GATES_TOURIST_H
#define GATES_TOURIST_H

typedef enum {
	REQUESTING_STORE_SLOT,
	WAITING_FOR_FREE_STORE_SLOT,
	OCCUPYING_STORE_SLOT,
} tourist_state;

typedef struct {
	int store_claimed;
	int is_deferred;
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
	// Resource-related variables
	world_resources *res;
	int free_store_slots;
	int total_store_slots;
} tourist;

extern tourist T;

void T_print_res();
void T_enter_store(int tourist);
void T_leave_store(int tourist);
#endif

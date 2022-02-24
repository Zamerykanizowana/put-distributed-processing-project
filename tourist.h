#ifndef GATES_TOURIST_H
#define GATES_TOURIST_H

typedef enum {
	WAITING_FOR_STORE,
	SHOPPING,
	WAITING_FOR_PSYCHIC,
	TRIPPING
} tourist_state;

typedef struct {
	int store_claimed;
} world_resources;

typedef struct {
	int psychic_claimed;
} world_resources_psychic;

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
	// If variable when_break_needed is 0, then process needs to wait until
	// psychic finish break
	world_resources_psychic *res_psychic;
	int when_break_needed;
	int total_psychic_slots;

	 
} tourist;

extern tourist T;

void T_print_res();
void T_enter_store(int tourist);
void T_leave_store(int tourist);
#endif

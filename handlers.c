#include <mpi.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include "msg.h"
#include "tourist.h"
#include "log.h"
#include "handlers.h"

void handle_req_store(int src, general_msg msg) {
	int tag;

	
	if (T.state == WAITING_FOR_STORE || T.state == SHOPPING) {
		if (incoming_event_happened_before(msg.clk, src)) {
			if (T.state == SHOPPING) {
				tag = NACK;
			} else {	
				tag = ACK;
				T_enter_store(src);
			}
		} else {
			tag = NACK;
		}
	} else {
		tag = ACK;
		T_enter_store(src);	
	}

	if (tag == ACK) {	
		log_info("Sending ACK to %d. My clk is %d", src, T.clk);
	} else {	
		log_info("Sending NACK to %d. My clk is %d", src, T.clk);
	}

	//T.clk++;

	general_msg new_msg = {T.clk};
	MPI_Send(&new_msg,
			sizeof(new_msg),
			MPI_BYTE,
			src,
			tag,
			MPI_COMM_WORLD
		);
}

// src is process number (incoming p_id)
void handle_ack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE || T.state == WAITING_FOR_PSYCHIC) {
		T.responses++;
	}

	switch (T.state) {
		case WAITING_FOR_STORE:
			T.res[src].store_claimed = 0;
			break;
		case WAITING_FOR_PSYCHIC:
			if (T.res_psychic[src].psychic_claimed == -1) {
				T.res_psychic[src].psychic_claimed = 0;
			}
			break;
		default:
			break;
	}

}

void handle_nack(int src, general_msg msg) {
	if (T.state == WAITING_FOR_STORE || T.state == WAITING_FOR_PSYCHIC) {
		T.responses++;
	}

	//TODO for Ola - need to check, what to do with that
	switch (T.state) {
		case WAITING_FOR_STORE:
			T_enter_store(src);
			break;
		case WAITING_FOR_PSYCHIC:
			T_enter_psychic(src);
			break;
		default:
			break;
	}
}

void handle_release_store(int tourist) {
	T_leave_store(tourist);
}

void handle_waiting_for_store_state() {
	// Process only once we've gathered responses from all nodes.
	if (T.responses == T.size - 2) {
		
		if (T.free_store_slots > 0) {
			// Consume responses (empty the stomach).
			T.responses = 0;
			T.clk++;
			T.state = SHOPPING;
			T_enter_store(T.rank);
			log_info("I'm shopping now! 🏪👀");

			pthread_t glue_fan;
			pthread_create(&glue_fan, NULL, do_the_shopping, NULL);
		} else {
			//T.state = WAITING_FOR_STORE;
			log_info("Grr, I have to wait... but I have all responses");
		}
	} else {
		log_info("Grr, I have to wait, I have only %d resp", T.responses);
	}
}

void *do_the_shopping(void *arg) {
	int duration = 3;

	log_info("I'll spend %d seconds in the store", duration);
	sleep(duration);
	T.state = WAITING_FOR_PSYCHIC;
	log_info("Shopping done! 🏪✅");
	
	T_leave_store(T.rank);

	release_store();

	enter_psychic_req();

	return NULL;
}

void handle_req_psychic(int src, general_msg msg) {
	int tag;

	if (T.state == WAITING_FOR_PSYCHIC) {
		if (incoming_event_happened_before(msg.clk, src) && 
				T.res_psychic[src].psychic_claimed != 0) {
			tag = ACK;
			log_info("Sending ACK_PSYCHIC to %d. My clk is %d, my st is %d", 
					src, T.clk, T.state);
			T_enter_psychic(src);
		} else {
			tag = NACK;
			log_info("Sending NACK_PSYCHIC to %d. My clk is %d, my st is %d", 
					src, T.clk, T.state);
			if (T.res_psychic[src].psychic_claimed == -1) {
				T.res_psychic[src].psychic_claimed = 0;
			}
		}
	} else {
		tag = ACK;
		log_info("Sending ACK_PSYCHIC to %d. My clk is %d, my st is %d", 
				src, T.clk, T.state);
		T_enter_psychic(src);
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

void handle_waiting_for_psychic() {
	if (T.responses == T.size - 2 || T.res_que[T.rank].psychic_queue != -1 ) {
		log_info("I have all responses about psychic!");
		T.res_psychic[T.rank].psychic_claimed = 0;
		// Save queque to local history
		// To have info which process entred before
		if (T.responses != 0) {
			for (int i = 1; i < T.size; i++) {
				T.res_que[i].psychic_queue = T.res_psychic[i].psychic_claimed;
			}
		}
		T_print_local_que();
		//T.responses = 0;
		//T.clk++;
		//T_enter_psychic(T.rank) ?? - nope
		log_info("My state of wbn: %d", T.when_break_needed);
		if (T.when_break_needed == 0) {
			T.responses = 0;
			handle_trying_to_enter();
		} else {
			if (T.when_break_needed % T.total_psychic_slots == 0 && T.responses != 0) {
				T.responses = 0;
				log_info("Psychic's break 😴");
				T.state = BREAK;
				pthread_t break_needed;
				pthread_create(&break_needed, NULL, do_break, NULL);
			} else {
				T.responses = 0;
				handle_trying_to_enter();
			}
		}
	} else {
		log_info("I have only %d responses about psychic...", T.responses);
	}

}

void *do_break(void *arg) {
	int duration = 4;
	log_info("Break takes %d seconds", duration);
	sleep(duration);
	log_info("Break done! 😴✅");
	T.state = WAITING_FOR_PSYCHIC;
	while (T.state == WAITING_FOR_PSYCHIC) {
		handle_trying_to_enter();
		sleep(1);
	}
	return NULL;
}

void handle_trying_to_enter() {
	int blocker = 0;
	for (int i = 1; i < T.size; i++) {
		if (T.res_que[i].psychic_queue == 1) {
			blocker++;
		}
	}
	if (blocker == 0) {
		T.clk++;
		T.state = TRIPPING;
		enter_psychic();
		pthread_t trip_fan;
		pthread_create(&trip_fan, NULL, do_the_trip, NULL);
	}
} 

void *do_the_trip(void *arg) {
	int duration = (rand() % (10 - 1 + 1)) + 1;
	log_info("I want to spend %d seconds in tunnel", duration);
	sleep(duration);
	T.state = WAITING_TO_EXIT;
	log_info("I'm ready to exit tunnel...");
	T_print_local_que();
	handle_wanting_to_exit();
	return NULL;
}

void handle_enter(int tourist) {
	if (T.res_que[tourist].psychic_queue == 1) {
		T.res_que[tourist].psychic_queue = 2;
	}
	T.res_psychic[tourist].psychic_claimed = 2;
	log_info("Handle entering tourist no. %d", tourist);
	T_print_res_psychic();
	T_print_local_que();

}

void handle_wanting_to_exit() {
	log_info("HERE in handle");
	int blocker = 0;
	for (int i = 1; i < T.size; i++) {
		if (T.res_que[i].psychic_queue == 2) {
			blocker++;
		}
	}
	if (blocker == 0 && T.state == WAITING_TO_EXIT) {
		log_info("HERE");
		T.clk++;
		T.state = EXITED;
		for (int i = 1; i < T.size; i++) {
			T.res_que[i].psychic_queue = -1;
		}
		release_psychic();

	} //else {
	//	sleep(2);
	//	handle_wanting_to_exit();
	//}
	
}

void handle_release_psychic(int tourist) {
	T.res_psychic[tourist].psychic_claimed = -1;
	T.res_que[tourist].psychic_queue = 0;
	log_debug("HELO FROM HANDLE");
	T_print_res_psychic();
	T_print_local_que();

}

#include <string.h>
#include "tourist.h"
#include "log.h"

tourist T;

void T_print_res() {
	char l[128] = {'\0'};
	char *l_ptr = l;

	for (int i = 1; i < T.size; i++) {
		l_ptr += sprintf(l_ptr, "P%d -- %d; ", 
				i, T.res[i].store_claimed);
	}

	log_info(l);
}

void T_print_res_psychic() {
	char l[128] = {'\0'};
	char *l_ptr = l;
	for (int i = 1; i < T.size; i++) {
		l_ptr += sprintf(l_ptr, "P%d_PS -- %d; ", i, T.res_psychic[i].psychic_claimed);
	
	}

	log_info(l);
}

void T_print_local_que() {
	char l[128] = {'\0'};
	char *l_ptr = l;
	l_ptr += sprintf("My local que: ");
	for (int i = 1; i < T.size; i++) {
		l_ptr += sprintf(l_ptr, "P%d_Q -- %d; ", i, T.res_que[i].psychic_queue);
	}
	log_info(l);
}

void T_enter_store(int tourist) {
	if (T.res[tourist].store_claimed == 0) {
		T.free_store_slots--;
		T.res[tourist].store_claimed = 1;
	//	if (tourist == T.rank) {
	//		log_info("I have entered the store! 🏪👀");
	//	} else {
	//		log_info("%d has entered the store! 🏪", tourist);
	//	}
	}
}

void T_leave_store(int tourist) {
	if (T.res[tourist].store_claimed == 1) {
		T.res[tourist].store_claimed = 0;
		T.free_store_slots++;
	}

	if (T.free_store_slots > T.total_store_slots) {
		log_error("Free store slots (%d) exceeded total store slots (%d)",
				T.free_store_slots, T.total_store_slots);
	}
}

void T_enter_psychic(int tourist) {
	if (T.res_psychic[tourist].psychic_claimed == 0) {
		T.res_psychic[tourist].psychic_claimed = 1;
		T.when_break_needed--;
		
	}
}

#include "tourist.h"
#include "log.h"

tourist T;

void T_print_res() {
	for (int i = 1; i < T.size; i++) {
		log_info("%d state might be %d", i, T.res[i].store_claimed);
	}	
}

void T_enter_store(int tourist) {
	if (T.res[tourist].store_claimed == 0) {
		T.free_store_slots--;
		T.res[tourist].store_claimed = 1;

		if (tourist == T.rank) {
			log_info("I have entered the store! 🏪👀");
		} else {
			log_info("%d has entered the store! 🏪", tourist);
		}
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

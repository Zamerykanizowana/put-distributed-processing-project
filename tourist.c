#include "tourist.h"

tourist T;

void T_enter_store(int tourist) {
	if (T.res[tourist].store_claimed == 0) {
		T.free_store_slots--;
		T.res[tourist].store_claimed = 1;
	}
}

void T_leave_store(int tourist) {
	T.free_store_slots++;
	T.res[tourist].store_claimed = 0;
}

#include "msg.h"
#include "tourist.h"

// Check if sender wins.
// In case of timestamp conflict, lower rank wins.
int incoming_event_happened_before(int incoming_clk, int incoming_rank) {
	return (T.clk > incoming_clk || 
			(T.clk == incoming_clk && 
			 incoming_rank < T.rank)
	       );
}

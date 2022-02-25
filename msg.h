#ifndef GATES_MESSAGE_H
#define GATES_MESSAGE_H

//TODO: establish whether we need REL_SHOP
typedef enum {
	REQ_STORE,
	REL_STORE,
	REQ_PSYCHIC,
	ENTER_TUNNEL,
	REL_PSYCHIC,
	ACK,
	NACK
} msg_tag;

typedef struct general_msg {
	int clk;
} general_msg;

int incoming_event_happened_before(int incoming_clk, int incoming_rank); 
void send_to_all(general_msg msg, msg_tag tag);
void print_size_rank();
void enter_store_req();
void release_store();
void enter_psychic_req();
void enter_psychic();
void release_psychic();
#endif

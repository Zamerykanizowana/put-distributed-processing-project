#ifndef GATES_MESSAGE_H
#define GATES_MESSAGE_H

//TODO: establish whether we need REL_SHOP
typedef enum {
	REQ_SHOP,
	REQ_PSYCHIC,
	REL_PSYCHIC,
	ACK,
	NACK
} msg_tag

typedef struct general_msg {
	int clk;
}

#endif

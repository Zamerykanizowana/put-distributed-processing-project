#ifndef GATES_HANDLERS_H
#define GATES_HANDLERS_H
#include "msg.h"

void handle_req_store(int src, general_msg msg);
void handle_ack(int src, general_msg msg);
void handle_nack(int src, general_msg msg);
#endif

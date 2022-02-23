#ifndef GATES_HANDLERS_H
#define GATES_HANDLERS_H
#include "msg.h"

void *do_the_shopping(void *arg);
void handle_req_store(int src, general_msg msg);
void handle_ack(int src, general_msg msg);
void handle_nack(int src, general_msg msg);
void handle_release_store(int tourist);
void handle_waiting_for_store_state(); 
void handle_waiting_for_free_store_slot();
#endif

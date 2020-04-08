#ifndef SERVER_H
#define SERVER_H

#include "buffer.h"
#include "connection.h"

void wait_init(struct sockaddr_in* addr);
void recv_video(cbuf_handle_t buf);
void run_server(int serverPort);

#endif // SERVER_H
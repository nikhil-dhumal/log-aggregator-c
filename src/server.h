#ifndef SERVER_H
#define SERVER_H

#include "civetweb.h"

extern struct mg_context *server_ctx;

struct mg_context *start_server(void);
void stop_server(void);
int handle_ping(struct mg_connection *conn, void *cbdata);
int handle_log(struct mg_connection *conn, void *cbdata);

#endif

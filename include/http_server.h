#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "civetweb.h"

extern struct mg_context *server_ctx;

struct mg_context *start_server(void);
void stop_server(void);

#endif

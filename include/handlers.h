#ifndef HANDLERS_H
#define HANDLERS_H

#include "civetweb.h"

int handle_health(struct mg_connection *conn, void *cbdata);
int handle_post_log(struct mg_connection *conn, void *cbdata);
int handle_get_logs(struct mg_connection *conn, void *cbdata);

#endif

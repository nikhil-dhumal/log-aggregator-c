#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cache.h"
#include "config.h"
#include "db.h"
#include "handlers.h"
#include "http_server.h"
#include "log_queue.h"
#include "log_worker.h"

struct mg_context *server_ctx = NULL;

static const char *server_options[] = {
    "listening_ports", SERVER_PORT,
    NULL};

int handle_logs(struct mg_connection *conn, void *cbdata)
{
    const char *method = mg_get_request_info(conn)->request_method;

    if (strcmp(method, "POST") == 0)
    {
        return handle_post_log(conn, cbdata);
    }
    else if (strcmp(method, "GET") == 0)
    {
        return handle_get_logs(conn, cbdata);
    }
    else
    {
        mg_printf(conn,
                  "HTTP/1.1 405 Method Not Allowed\r\n"
                  "Content-Type: application/json\r\n"
                  "Connection: close\r\n\r\n"
                  "{\"error\":\"Method not allowed\"}\n");
        return 405;
    }
}

struct mg_context *start_server(void)
{
    server_ctx = mg_start(NULL, NULL, server_options);
    if (server_ctx == NULL)
    {
        fprintf(stderr, "[server] ERROR: Failed to start CivetWeb\n");
        return NULL;
    }
    mg_set_request_handler(server_ctx, "/ping", handle_health, NULL);
    mg_set_request_handler(server_ctx, "/logs", handle_logs, NULL);
    if (cache_init(CACHE_SIZE) != 0)
    {
        fprintf(stderr, "[server] Cache init failed. Exiting.\n");
        stop_server();
        return NULL;
    }
    if (db_init() != 0)
    {
        cache_destroy();
        stop_server();
        return NULL;
    }
    if (queue_init() != 0)
    {
        fprintf(stderr, "[server] ERROR: failed to init queue\n");
        cache_destroy();
        db_close();
        stop_server();
        return NULL;
    }
    worker_start();
    return server_ctx;
}

void stop_server(void)
{
    if (server_ctx != NULL)
    {
        mg_stop(server_ctx);
        server_ctx = NULL;
    }
    queue_shutdown();
    worker_stop();
    queue_destroy();
    db_close();
    cache_destroy();
}

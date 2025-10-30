#include "server.h"

struct mg_context *server_ctx = NULL;

static const char *server_options[] = {
    "listening_ports", "8080",
    // TODO: add future options here (thread count, etc.)
    NULL};

int handle_ping(struct mg_connection *conn, void *cbdata)
{
  const char *res = "{\"status\":\"ok\"}\n";
  mg_printf(conn, "HTTP/1.1 200 OK\r\n");
  mg_printf(conn, "Content-Type: application/json\r\n");
  mg_printf(conn, "Connection: close\r\n");
  mg_printf(conn, "\r\n");
  mg_printf(conn, "%s\r\n", res);
  return 200;
}

struct mg_context *start_server(void)
{
  server_ctx = mg_start(NULL, NULL, server_options);
  if (server_ctx == NULL)
  {
    fprintf(stderr, "ERROR: Failed to start CivetWeb server on port 8080\n");
    return NULL;
  }
  mg_set_request_handler(server_ctx, "/ping", handle_ping, NULL);
  return server_ctx;
}

void stop_server(void)
{
  if (server_ctx != NULL)
  {
    mg_stop(server_ctx);
    server_ctx = NULL;
  }
}

#include <stdlib.h>
#include <string.h>
#include "server.h"

struct mg_context *server_ctx = NULL;

static const char *server_options[] = {
    "listening_ports", "8080",
    // TODO: add future options here (thread count, etc.)
    NULL};

int handle_ping(struct mg_connection *conn, void *cbdata)
{
  (void)cbdata;
  const char *res = "{\"status\":\"ok\"}\n";
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n"
            "%s\n",
            res);
  return 200;
}

int handle_log(struct mg_connection *conn, void *cbdata)
{
  (void)cbdata;
  const struct mg_request_info *req = mg_get_request_info(conn);

  if (strcmp(req->request_method, "POST") != 0)
  {
    mg_printf(conn,
              "HTTP/1.1 405 Method Not Allowed\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"error\":\"Use POST\"}\n");
    return 405;
  }

  long long len = req->content_length;

  if (len <= 0)
  {
    mg_printf(conn,
              "HTTP/1.1 400 Bad Request\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"error\":\"Empty body\"}\n");
    return 400;
  }

  char *body = malloc(len + 1);
  mg_read(conn, body, len);
  body[len] = '\0';

  printf("Log received: %s\n", body);

  free(body);

  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n"
            "{\"status\":\"accepted\"}\n");

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
  mg_set_request_handler(server_ctx, "/log", handle_log, NULL);
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

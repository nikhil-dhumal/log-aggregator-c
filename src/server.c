#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "cJSON.h"

struct mg_context *server_ctx = NULL;

static const char *server_options[] = {
    "listening_ports", "8080",
    // TODO: add future options here (thread count, etc.)
    NULL};

int handle_ping(struct mg_connection *conn, void *cbdata)
{
  (void)cbdata;
  mg_printf(conn,
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: application/json\r\n"
            "Connection: close\r\n\r\n"
            "{\"status\":\"ok\"}\n");
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

  cJSON *json = cJSON_Parse(body);

  if (json == NULL)
  {
    mg_printf(conn,
              "HTTP/1.1 400 Bad Request\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"error\":\"Invalid JSON\"}\n");
    return 400;
  }

  cJSON *level = cJSON_GetObjectItemCaseSensitive(json, "level");
  cJSON *message = cJSON_GetObjectItemCaseSensitive(json, "message");

  if (!cJSON_IsString(level) || !cJSON_IsString(message))
  {
    mg_printf(conn,
              "HTTP/1.1 400 Bad Request\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"error\":\"Fields 'level' and 'message' must be strings\"}\n");
    cJSON_Delete(json);
    free(body);
    return 400;
  }

  printf("LOG: Level=%s message=%s\n", level->valuestring, message->valuestring);

  cJSON_Delete(json);
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

#include <stdlib.h>
#include <string.h>
#include "server.h"
#include "cJSON.h"
#include "log_entry.h"
#include "queue.h"
#include "worker.h"
#include "storage.h"
#include "cache.h"

#define DEFAULT_GET_LIMIT 10
#define DEFAULT_CACHE_SIZE 1000

struct mg_context *server_ctx = NULL;

static const char *server_options[] = {
    "listening_ports", "8080",
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

int handle_post_log(struct mg_connection *conn, void *cbdata)
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

    log_entry entry;
    strncpy(entry.level, level->valuestring, sizeof(entry.level) - 1);
    entry.level[sizeof(entry.level) - 1] = '\0';
    strncpy(entry.message, message->valuestring, sizeof(entry.message) - 1);
    entry.message[sizeof(entry.message) - 1] = '\0';
    entry.timestamp = time(NULL);

    if (queue_push(&entry) != 0)
    {
        mg_printf(conn,
                  "HTTP/1.1 503 Service Unavailable\r\n"
                  "Content-Type: application/json\r\n"
                  "Connection: close\r\n\r\n"
                  "{\"error\":\"Server shutting down\"}\n");
        cJSON_Delete(json);
        free(body);
        return 503;
    }

    cJSON_Delete(json);
    free(body);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"status\":\"accepted\"}\n");
    return 200;
}

int handle_get_logs(struct mg_connection *conn, void *cbdata)
{
    (void)cbdata;
    const struct mg_request_info *req = mg_get_request_info(conn);

    if (strcmp(req->request_method, "GET") != 0)
    {
        mg_printf(conn,
                  "HTTP/1.1 405 Method Not Allowed\r\n"
                  "Content-Type: application/json\r\n"
                  "Connection: close\r\n\r\n"
                  "{\"error\":\"Use GET\"}\n");
        return 405;
    }

    int limit = DEFAULT_GET_LIMIT;
    char limit_str[16] = {0};

    if (req->query_string != NULL && mg_get_var(req->query_string, strlen(req->query_string), "limit", limit_str, sizeof(limit_str)) > 0)
    {
        limit = atoi(limit_str);
        if (limit <= 0)
        {
            limit = DEFAULT_GET_LIMIT;
        }
    }

    log_entry *logs = malloc(limit * sizeof(log_entry));

    int count = cache_get_last(limit, logs);

    if (count < limit)
    {
        count = storage_read_last(limit, logs);
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();

    for (int i = 0; i < count; i++)
    {
        cJSON *item = cJSON_CreateObject();
        cJSON_AddNumberToObject(item, "timestamp", logs[i].timestamp);
        cJSON_AddStringToObject(item, "level", logs[i].level);
        cJSON_AddStringToObject(item, "message", logs[i].message);
        cJSON_AddItemToArray(array, item);
    }

    cJSON_AddItemToObject(root, "logs", array);
    char *json_str = cJSON_PrintUnformatted(root);

    free(logs);

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "%s",
              json_str);

    cJSON_Delete(root);
    free(json_str);

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
    mg_set_request_handler(server_ctx, "/log", handle_post_log, NULL);
    mg_set_request_handler(server_ctx, "/logs", handle_get_logs, NULL);
    cache_init(DEFAULT_CACHE_SIZE);
    if (storage_init() != 0)
    {
        return NULL;
    }
    queue_init();
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
    storage_close();
    cache_destroy();
}

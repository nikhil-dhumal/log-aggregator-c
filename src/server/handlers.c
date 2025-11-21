#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "cache.h"
#include "config.h"
#include "db.h"
#include "handlers.h"
#include "log_entry.h"
#include "log_queue.h"

int handle_health(struct mg_connection *conn, void *cbdata)
{
    (void)cbdata;
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"status\":\"healthy\"}\n");
    return 200;
}

int handle_post_log(struct mg_connection *conn, void *cbdata)
{
    (void)cbdata;
    const struct mg_request_info *req = mg_get_request_info(conn);
    
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
    int total = 0;

    while (total < len)
    {
        int curr = mg_read(conn, body + total, len - total);
        if (curr <= 0)
        {
            break;
        }
        total += curr;
    }
    body[total] = '\0';

    cJSON *json = cJSON_Parse(body);

    if (json == NULL)
    {
        mg_printf(conn,
                  "HTTP/1.1 400 Bad Request\r\n"
                  "Content-Type: application/json\r\n"
                  "Connection: close\r\n\r\n"
                  "{\"error\":\"Invalid JSON\"}\n");
        free(body);
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

    for (char *p = entry.level; *p; p++)
    {
        *p = toupper(*p);
    }

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

    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: application/json\r\n"
              "Connection: close\r\n\r\n"
              "{\"status\":\"accepted\"}\n");
    cJSON_Delete(json);
    free(body);
    return 200;
}

int handle_get_logs(struct mg_connection *conn, void *cbdata)
{
    (void)cbdata;
    const struct mg_request_info *req = mg_get_request_info(conn);

    int limit = DEFAULT_GET_LIMIT;
    char limit_str[16] = {0};

    if (req->query_string != NULL && mg_get_var(req->query_string, strlen(req->query_string), "limit", limit_str, sizeof(limit_str)) > 0)
    {
        limit = atoi(limit_str);
        if (limit <= 0)
        {
            limit = DEFAULT_GET_LIMIT;
        }
        if (limit > 1000)
        {
            limit = 1000;
        }
    }

    char level_buf[16] = {0};
    char *level = NULL;

    if (req->query_string != NULL && mg_get_var(req->query_string, strlen(req->query_string), "level", level_buf, sizeof(level_buf)) > 0)
    {
        level = level_buf;
    }

    if (level)
    {
        for (char *p = level; *p; p++)
        {
            *p = toupper(*p);
        }
    }

    log_entry *logs = malloc(limit * sizeof(log_entry));

    if (!logs)
    {
        mg_printf(conn,
                  "HTTP/1.1 500 Internal Server Error\r\n"
                  "Content-Type: application/json\r\n"
                  "Connection: close\r\n\r\n"
                  "{\"error\":\"Out of memory\"}");
        return 500;
    }

    int got_from_cache = cache_get_last(limit, level, logs);
    int total = got_from_cache;

    if (got_from_cache < limit)
    {
        int need = limit - got_from_cache;
        int got_from_db = db_read_range(need, got_from_cache, level, logs + got_from_cache);
        total += got_from_db;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON *array = cJSON_CreateArray();

    for (int i = 0; i < total; i++)
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

#include <libpq-fe.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "db.h"
#include "log_entry.h"

static PGconn *pg_conn_write = NULL;

int db_init(void)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (conn_info == NULL)
    {
        fprintf(stderr, "[db] ERROR: Database connection failed\n");
        return -1;
    }

    PGconn *conn = PQconnectdb(conn_info);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "[db] ERROR: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return -1;
    }

    printf("[db] Connected\n");

    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS logs("
        "id SERIAL PRIMARY KEY,"
        "timestamp BIGINT NOT NULL,"
        "level VARCHAR(16) NOT NULL,"
        "message VARCHAR(512) NOT NULL"
        ");";

    PGresult *res = PQexec(conn, create_table_query);

    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "[db] ERROR: %s\n", PQerrorMessage(conn));
        if (res)
        {
            PQclear(res);
        }
        PQfinish(conn);
        return -1;
    }

    PQclear(res);
    PQfinish(conn);
    return 0;
}

int db_init_writer(void)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (conn_info == NULL)
    {
        fprintf(stderr, "[db] ERROR: Database connection failed\n");
        return -1;
    }

    pg_conn_write = PQconnectdb(conn_info);
    if (PQstatus(pg_conn_write) != CONNECTION_OK)
    {
        fprintf(stderr, "[db] ERROR: %s\n", PQerrorMessage(pg_conn_write));
        PQfinish(pg_conn_write);
        pg_conn_write = NULL;
        return -1;
    }

    return 0;
}

int db_write(log_entry *entry)
{
    char ts[32];
    snprintf(ts, sizeof(ts), "%ld", entry->timestamp);

    const char *param_values[3] = {ts, entry->level, entry->message};

    PGresult *res = PQexecParams(
        pg_conn_write,
        "INSERT INTO logs (timestamp, level, message) VALUES ($1, $2, $3)",
        3,
        NULL,
        param_values,
        NULL,
        NULL,
        0);

    if (!res || PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "[db] ERROR: %s\n", PQerrorMessage(pg_conn_write));
        if (res) 
        {
            PQclear(res);
        }
        return -1;
    }

    PQclear(res);
    return 0;
}

PGconn *db_get_connection(void)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (!conn_info)
    {
        return NULL;
    }

    PGconn *conn = PQconnectdb(conn_info);
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "[db] ERROR: %s\n", PQerrorMessage(conn));
        PQfinish(conn);
        return NULL;
    }

    return conn;
}

int db_read_range(int limit, int offset, const char *level, log_entry *buffer)
{
    if (limit <= 0 || offset < 0)
    {
        return 0;
    }

    char limit_str[16], offset_str[16];
    snprintf(limit_str, sizeof(limit_str), "%d", limit);
    snprintf(offset_str, sizeof(offset_str), "%d", offset);

    PGconn *pg_conn_read = db_get_connection();

    if (!pg_conn_read)
    {
        return -1;
    }

    PGresult *res = NULL;

    if (level == NULL)
    {
        const char *param_values[2] = {limit_str, offset_str};
        res = PQexecParams(
            pg_conn_read,
            "SELECT timestamp, level, message FROM logs ORDER BY id DESC LIMIT $1 OFFSET $2",
            2,
            NULL,
            param_values,
            NULL,
            NULL,
            0);
    }
    else
    {
        const char *param_values[3] = {level, limit_str, offset_str};
        res = PQexecParams(
            pg_conn_read,
            "SELECT timestamp, level, message FROM logs WHERE level = $1 ORDER BY id DESC LIMIT $2 OFFSET $3",
            3,
            NULL,
            param_values,
            NULL,
            NULL,
            0);
    }

    if (!res || PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "[db] 1 ERROR: %s\n", PQerrorMessage(pg_conn_read));
        if (res)
        {
            PQclear(res);
        }
        return -1;
    }

    int rows = PQntuples(res);

    for (int i = 0; i < rows; i++)
    {
        buffer[i].timestamp = atoll(PQgetvalue(res, i, 0));
        strncpy(buffer[i].level, PQgetvalue(res, i, 1), sizeof(buffer[i].level) - 1);
        buffer[i].level[sizeof(buffer[i].level) - 1] = '\0';
        strncpy(buffer[i].message, PQgetvalue(res, i, 2), sizeof(buffer[i].message) - 1);
        buffer[i].message[sizeof(buffer[i].message) - 1] = '\0';
    }

    PQclear(res);
    PQfinish(pg_conn_read);
    return rows;
}

void db_close(void)
{
    if (pg_conn_write)
    {
        PQfinish(pg_conn_write);
        pg_conn_write = NULL;
    }
}

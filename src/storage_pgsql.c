#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "log_entry.h"

static PGconn *pg_conn = NULL;

int storage_init(const char *filename)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (conn_info == NULL)
    {
        fprintf(stderr, "ERROR: Database connection failed\n");
        return -1;
    }

    pg_conn = PQconnectdb(conn_info);
    if (PGstatus(pg_conn) != CONNECTION_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn));
        PQfinish(pg_conn);
        return -1;
    }

    return 0;
}

int storage_write(log_entry *entry)
{
    char ts[32];
    snprintf(ts, sizeof(ts), "%ld", entry->timestamp);

    const char *param_values[3] = {ts, entry->level, entry->message};

    PGresult *res = PQexecParams(
        pg_conn,
        "INSERT INTO logs (timestamp, level, message) VALUES ($1, $2, $3)",
        3,
        NULL,
        param_values,
        NULL,
        NULL,
        0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int storage_read_last(int limit, log_entry *buffer)
{
    char limit_str[6];
    snprintf(limit_str, sizeof(limit_str), "%d", limit);

    const char *param_values[1] = {limit_str};

    PGresult *res = PQexecParams(
        pg_conn,
        "SELECT timestamp, level, message FROM logs ORDER BY id DESC LIMIT $1",
        1,
        NULL,
        param_values,
        NULL,
        NULL,
        0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn));
        PQclear(res);
        return 0;
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
    return rows;
}

void storage_close(void)
{
    PQfinish(pg_conn);
    pg_conn = NULL;
}
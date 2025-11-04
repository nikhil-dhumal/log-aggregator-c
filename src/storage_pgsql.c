#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libpq-fe.h>
#include "log_entry.h"

static PGconn *pg_conn_read = NULL;
static PGconn *pg_conn_write = NULL;

int storage_init(void)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (conn_info == NULL)
    {
        fprintf(stderr, "ERROR: Database connection failed\n");
        return -1;
    }

    pg_conn_read = PQconnectdb(conn_info);
    if (PQstatus(pg_conn_read) != CONNECTION_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn_read));
        PQfinish(pg_conn_read);
        return -1;
    }

    printf("Database connected\n");

    const char *create_table_query =
        "CREATE TABLE IF NOT EXISTS logs("
        "id SERIAL PRIMARY KEY,"
        "timestampe BIGINT NOT NULL,"
        "level TEXT NOT NULL,"
        "message TEXT NOT NULL"
        ");";

    PGresult *res = PQexec(pg_conn_read, create_table_query);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn_read));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int storage_init_writer(void)
{
    const char *conn_info = getenv("LOGDB_CONN");
    if (conn_info == NULL)
    {
        fprintf(stderr, "ERROR: Database connection failed\n");
        return -1;
    }

    pg_conn_write = PQconnectdb(conn_info);
    if (PQstatus(pg_conn_write) != CONNECTION_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn_write));
        PQfinish(pg_conn_write);
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
        pg_conn_write,
        "INSERT INTO logs (timestamp, level, message) VALUES ($1, $2, $3)",
        3,
        NULL,
        param_values,
        NULL,
        NULL,
        0);

    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn_write));
        PQclear(res);
        return -1;
    }

    PQclear(res);
    return 0;
}

int storage_read_range(int limit, int offset, log_entry *buffer)
{
    char limit_str[6], offset_str[6];
    snprintf(limit_str, sizeof(limit_str), "%d", limit);
    snprintf(offset_str, sizeof(offset_str), "%d", offset);

    const char *param_values[2] = {limit_str, offset_str};

    PGresult *res = PQexecParams(
        pg_conn_read,
        "SELECT timestamp, level, message FROM logs ORDER BY id DESC LIMIT $1 OFFSET $2",
        2,
        NULL,
        param_values,
        NULL,
        NULL,
        0);

    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        fprintf(stderr, "DB ERROR: %s\n", PQerrorMessage(pg_conn_read));
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
    if (pg_conn_read)
    {
        PQfinish(pg_conn_read);
        pg_conn_read = NULL;
    }
    if (pg_conn_write)
    {
        PQfinish(pg_conn_write);
        pg_conn_write = NULL;
    }
}
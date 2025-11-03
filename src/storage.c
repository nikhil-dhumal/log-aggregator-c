#include <stdio.h>
#include <pthread.h>
#include "storage.h"

pthread_mutex_t lock;
static const char *storage_filename = NULL;
static FILE *storage_file = NULL;

int storage_init(const char *filename)
{
    storage_filename = filename;
    storage_file = fopen(storage_filename, "a");
    if (storage_file == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file: %s\n", storage_filename);
        return -1;
    }
    pthread_mutex_init(&lock, NULL);
    return 0;
}

void storage_write(log_entry *entry)
{
    pthread_mutex_lock(&lock);
    fprintf(storage_file, "%ld %s %s\n", entry->timestamp, entry->level, entry->message);
    fflush(storage_file);
    pthread_mutex_unlock(&lock);
}

int storage_read_last(int limit, log_entry *buffer)
{
    FILE *storage_file_reader = fopen(storage_filename, "r");

    if (storage_file_reader == NULL)
    {
        fprintf(stderr, "ERROR: Failed to open file: %s\n", storage_filename);
        return -1;
    }

    fseek(storage_file_reader, 0, SEEK_END);

    long pos = ftell(storage_file_reader) - 1;

    if (pos < 0)
    {
        fclose(storage_file_reader);
        return 0;
    }

    while (pos > 0)
    {
        fseek(storage_file_reader, pos, SEEK_SET);
        if (fgetc(storage_file_reader) != '\n')
        {
            break;
        }
        pos--;
    }

    int entries_found = 0;

    while (pos >= 0 && entries_found < limit)
    {
        fseek(storage_file_reader, pos, SEEK_SET);
        if (fgetc(storage_file_reader) == '\n')
        {
            entries_found++;
        }
        pos--;
    }

    if (pos < 0)
    {
        pos = 0;
    }
    else
    {
        pos += 2;
    }

    fseek(storage_file_reader, pos, SEEK_SET);

    char line[512];
    int count = 0;

    while (count < entries_found && fgets(line, sizeof(line), storage_file_reader))
    {
        sscanf(line, "%ld %15s %[^\n]", &buffer[count].timestamp, buffer[count].level, buffer[count].message);
        count++;
    }

    fclose(storage_file_reader);

    return count;
}

void storage_close(void)
{
    fclose(storage_file);
    pthread_mutex_destroy(&lock);
}
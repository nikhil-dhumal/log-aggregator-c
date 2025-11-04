#ifndef STORAGE_H
#define STORAGE_H

#include "log_entry.h"

int storage_init(void);
int storage_write(log_entry *entry);
int storastorage_read_last(int limit, log_entry *buffer);
void storage_close(void);

#endif
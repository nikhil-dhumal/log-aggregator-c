#ifndef STORAGE_H
#define STORAGE_H

#include "log_entry.h"

int storage_init(void);
int storage_init_writer(void);
int storage_write(log_entry *entry);
int storage_read_range(int limit, int offset, log_entry *buffer);
void storage_close(void);

#endif
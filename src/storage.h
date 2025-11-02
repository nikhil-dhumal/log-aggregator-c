#ifndef STORAGE_H
#define STORAGE_H

#include "log_entry.h"

int storage_init(const char *filename);
void storage_write(log_entry *entry);
void storage_close(void);

#endif
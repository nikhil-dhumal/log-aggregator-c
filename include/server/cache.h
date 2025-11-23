#ifndef CACHE_H
#define CACHE_H

#include "log_entry.h"

int cache_init(int size);
void cache_destroy(void);
void cache_insert(log_entry *entry);
int cache_get_range(int limit, int offset, const char *level, log_entry *buffer);

#endif

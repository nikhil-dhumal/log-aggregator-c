#ifndef QUEUE_H
#define QUEUE_H

#include "log_entry.h"

void queue_init(void);
int queue_push(log_entry *entry);
int queue_pop(log_entry *out);
void queue_destroy(void);

#endif
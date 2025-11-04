#ifndef LOG_QUEUE_H
#define LOG_QUEUE_H

#include "log_entry.h"

int queue_init(void);
int queue_push(log_entry *entry);
int queue_pop(log_entry *out);
void queue_shutdown(void);
void queue_destroy(void);

#endif

#ifndef DB_H
#define DB_H

#include "log_entry.h"

int db_init(void);
int db_init_writer(void);
int db_write(log_entry *entires, int count);
int db_read_range(int limit, int offset, const char *level, log_entry *buffer);
void db_close(void);

#endif

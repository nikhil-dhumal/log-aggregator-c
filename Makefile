CC = gcc
CFLAGS = -Wall -Wextra -O2 -Ithird-party/civetweb-1.16/include -Ithird-party/cjson -I/usr/include/postgresql -DNO_SSL
LDFLAGS = -lpthread -lpq

# STORAGE = src/storage.c
STORAGE = src/storage_pgsql.c

SRC = src/main.c src/server.c src/queue.c src/worker.c $(STORAGE) src/cache.c \
      third-party/civetweb-1.16/src/civetweb.c third-party/cjson/cJSON.c

TARGET = log_server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

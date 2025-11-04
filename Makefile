CC = gcc
CFLAGS = -Wall -Wextra -O2 \
         -Iinclude \
         -Ithird-party/civetweb-1.16/include \
         -Ithird-party/cjson \
         -I/usr/include/postgresql \
         -DNO_SSL

LDFLAGS = -lpthread -lpq

SRC = src/main.c \
      src/http_server.c \
      src/handlers.c \
      src/log_queue.c \
      src/log_worker.c \
      src/cache.c \
      src/db_postgres.c \
      third-party/civetweb-1.16/src/civetweb.c \
      third-party/cjson/cJSON.c

TARGET = log_server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

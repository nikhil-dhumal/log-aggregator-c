CC = gcc
CFLAGS = -Wall -Wextra -O2 \
         -Iinclude/server \
         -Iinclude/client \
         -Ithird-party/civetweb-1.16/include \
         -Ithird-party/cjson \
         -I/usr/include/postgresql \
         -DNO_SSL

LDFLAGS = -lpthread -lpq -lcurl

SERVER_SRC = src/server/main.c \
      src/server/http_server.c \
      src/server/handlers.c \
      src/server/log_queue.c \
      src/server/log_worker.c \
      src/server/cache.c \
      src/server/db_postgres.c \
      third-party/civetweb-1.16/src/civetweb.c \
      third-party/cjson/cJSON.c

SERVER_TARGET = log_server

CLIENT_SRC = src/client/main.c \
      src/client/loadgen.c \
      src/client/loadgen_worker.c

CLIENT_TARGET = loadgen_client

all: $(SERVER_TARGETTARGET) $(CLIENT_TARGET)

$(SERVER_TARGET): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) $(SERVER_SRC) $(LDFLAGS)

$(CLIENT_TARGET): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) $(CLIENT_SRC) $(LDFLAGS)

clean:
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)

run_server: $(SERVER_TARGET)
	./$(SERVER_TARGET)

run_client: $(CLIENT_TARGET)
	./$(CLIENT_TARGET)

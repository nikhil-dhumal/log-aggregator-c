CC = gcc
CFLAGS = -Wall -Wextra -O2 -Ithird-party/civetweb-1.16/include -Ithird-party/cjson -DNO_SSL
LDFLAGS = -lpthread

SRC = src/main.c src/server.c third-party/civetweb-1.16/src/civetweb.c third-party/cjson/cJSON.c
TARGET = log_server

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC) $(LDFLAGS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

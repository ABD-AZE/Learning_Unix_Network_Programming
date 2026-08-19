CC = gcc
CFLAGS = -Wall -Wextra -std=c99
LDFLAGS = -lssl -lcrypto

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS)

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client received_*

.PHONY: all clean

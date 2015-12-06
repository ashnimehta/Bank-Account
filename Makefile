CC = gcc
CFLAGS = -g -D_GNU_SOURCE -D_POSIX_C_SOURCE -Wall -Werror -std=c99 -pedantic -pthread

all: client server

client: client.c
	$(CC) $(CFLAGS) -o client client.c client.h utils.h

server: server.c
	$(CC) $(CFLAGS) -o server server.c server.h utils.h


clean:
	-rm -rf client.dSYM
	-rm -rf server.dSYM
	-rm client
	-rm server

	
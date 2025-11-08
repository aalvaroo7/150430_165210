CC=gcc
CFLAGS=-O2 -Wall -Wextra -std=c11 -pthread

all: servidorProcTCP servidorHebTCP servidorUDP cliente

ops.o: ops.c ops.h
	$(CC) $(CFLAGS) -c ops.c

servidorProcTCP: server_proc_tcp.c ops.o
	$(CC) $(CFLAGS) -o servidorProcTCP server_proc_tcp.c ops.o

servidorHebTCP: server_thread_tcp.c ops.o
	$(CC) $(CFLAGS) -o servidorHebTCP server_thread_tcp.c ops.o

servidorUDP: server_udp.c ops.o
	$(CC) $(CFLAGS) -o servidorUDP server_udp.c ops.o

cliente: client.c
	$(CC) $(CFLAGS) -o cliente client.c

clean:
	rm -f *.o servidorProcTCP servidorHebTCP servidorUDP cliente

CC = gcc
CFLAGS = -Wall -Wextra -std=c99

all: programme

programme: structure.o main.o
	$(CC) $(CFLAGS) -o programme structure.o main.o

structure.o: structure.c structure.h
	$(CC) $(CFLAGS) -c structure.c

main.o: main.c structure.h
	$(CC) $(CFLAGS) -c main.c

default-run:
	./programme 02-rete.txt 02-veicoli.txt 02-chiamate.txt
clean:
	rm -f *.o programme


CFLAGS = -g -std=c99

default: all

all: *.c
	gcc $(CFLAGS) -c ss.c -o ss.o	
	gcc $(CFLAGS) -c lru.c -o lru.o
	gcc $(CFLAGS) -c main.c -o main.o
	gcc -o ss ss.o lru.o main.o 
clean:
	rm -f *.o
	rm -f ss

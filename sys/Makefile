CC=gcc
CFLAGS=-Wall -O2

all:
	$(CC) -c $(CFLAGS) exit.c -o exit.o
	$(CC) -c $(CFLAGS) log.c -o log.o
	$(CC) -c $(CFLAGS) xalloc.c -o xalloc.o
	$(CC) -c $(CFLAGS) communication.c -o communication.o
	$(CC) -c $(CFLAGS) processtitle.c -o processtitle.o
	$(CC) -c $(CFLAGS) sighandlers.c -o sighandlers.o
	$(CC) -c $(CFLAGS) users.c -o users.o

gdb:
	$(CC) -c -ggdb exit.c -o exit.o
	$(CC) -c -ggdb log.c -o log.o
	$(CC) -c -ggdb xalloc.c -o xalloc.o
	$(CC) -c -ggdb communication.c -o communication.o
	$(CC) -c -ggdb processtitle.c -o processtitle.o
	$(CC) -c -ggdb sighandlers.c -o sighandlers.o
	$(CC) -c -ggdb users.c -o users.o

debug:
	$(CC) -c -DDEBUG exit.c -o exit.o
	$(CC) -c -DDEBUG log.c -o log.o
	$(CC) -c -DDEBUG xalloc.c -o xalloc.o
	$(CC) -c -DDEBUG communication.c -o communication.o
	$(CC) -c -DDEBUG processtitle.c -o processtitle.o
	$(CC) -c -DDEBUG sighandlers.c -o sighandlers.o
	$(CC) -c -DDEBUG users.c -o users.o

debugdb:
	$(CC) -c -ggdb -DDEBUG exit.c -o exit.o
	$(CC) -c -ggdb -DDEBUG log.c -o log.o
	$(CC) -c -ggdb -DDEBUG xalloc.c -o xalloc.o
	$(CC) -c -ggdb -DDEBUG communication.c -o communication.o
	$(CC) -c -ggdb -DDEBUG processtitle.c -o processtitle.o
	$(CC) -c -ggdb -DDEBUG sighandlers.c -o sighandlers.o
	$(CC) -c -ggdb -DDEBUG users.c -o users.o

clean:
	rm -f *.o

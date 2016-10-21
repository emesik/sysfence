CC=gcc
LDFLAGS=
CFLAGS=-Wall -O2
objects=conditions.o getstats.o mainloop.o cp2memory.o datastruct.o sysfence.o
parseopt=parseopt/{confread,lex,parse}.o
sys=sys/{exit,xalloc,log,communication,sighandlers,processtitle,users}.o

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

sysfence: $(objects)
	make -C parseopt/ all
	make -C sys/ all
	$(CC) $(CFLAGS) $(LDFLAGS) -o sysfence $(objects) $(parseopt) $(sys)
	strip sysfence

all: sysfence

gdb:
	$(CC) -ggdb -c conditions.c -o conditions.o
	$(CC) -ggdb -c getstats.c -o getstats.o
	$(CC) -ggdb -c datastruct.c -o datastruct.o
	make -C parseopt/ gdb
	make -C sys/ gdb
	$(CC) -ggdb -c mainloop.c -o mainloop.o
	$(CC) -ggdb -c cp2memory.c -o cp2memory.o
	$(CC) -ggdb -c sysfence.c -o sysfence.o
	$(CC) -ggdb -o sysfence ${objects} ${parseopt} ${sys}

debug:
	$(CC) -DDEBUG -c conditions.c -o conditions.o
	$(CC) -DDEBUG -c getstats.c -o getstats.o
	$(CC) -DDEBUG -c datastruct.c -o datastruct.o
	make -C parseopt/ debug
	make -C sys/ debug
	$(CC) -DDEBUG -c mainloop.c -o mainloop.o
	$(CC) -DDEBUG -c cp2memory.c -o cp2memory.o
	$(CC) -DDEBUG -c sysfence.c -o sysfence.o
	$(CC) -o sysfence ${objects} ${parseopt} ${sys}

debugdb:
	$(CC) -DDEBUG -ggdb -c conditions.c -o conditions.o
	$(CC) -DDEBUG -ggdb -c getstats.c -o getstats.o
	$(CC) -DDEBUG -ggdb -c datastruct.c -o datastruct.o
	make -C parseopt/ debugdb
	make -C sys/ debugdb
	$(CC) -DDEBUG -ggdb -c mainloop.c -o mainloop.o
	$(CC) -DDEBUG -ggdb -c cp2memory.c -o cp2memory.o
	$(CC) -DDEBUG -ggdb -c sysfence.c -o sysfence.o
	$(CC) -ggdb -o sysfence ${objects} ${parseopt} ${sys}

clean:
	make -C parseopt/ clean
	make -C sys/ clean
	rm -f *.o sysfence

install:
	install -d $(DESTDIR)$(PREFIX)/bin
	install -d $(DESTDIR)/usr/share/doc/sysfence
	install -m 0755 sysfence $(DESTDIR)$(PREFIX)/bin
	install -m 0644 README doc/example.conf $(DESTDIR)/usr/share/doc/sysfence

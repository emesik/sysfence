all:
	gcc -c lex.c -o lex.o
	gcc -c parse.c -o parse.o
	gcc -c confread.c -o confread.o

gdb:
	gcc -c -ggdb lex.c -o lex.o
	gcc -c -ggdb parse.c -o parse.o
	gcc -c -ggdb confread.c -o confread.o

debug:
	gcc -c -DDEBUG lex.c -o lex.o
	gcc -c -DDEBUG parse.c -o parse.o
	gcc -c -DDEBUG confread.c -o confread.o

debugdb:
	gcc -c -ggdb -DDEBUG lex.c -o lex.o
	gcc -c -ggdb -DDEBUG parse.c -o parse.o
	gcc -c -ggdb -DDEBUG confread.c -o confread.o

clean:
	rm -f *.o

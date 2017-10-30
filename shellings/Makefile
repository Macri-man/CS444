shell: shell.o parse.o
	gcc -g -o shell shell.o parse.o -lreadline
shell.o: shell.c parse.h
	gcc -g -c shell.c 
parse.o: parse.c parse.h
	gcc -g -c parse.c
clean:
	rm -f shell *~ *.o

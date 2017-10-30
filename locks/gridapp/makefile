
CCC ?= gcc
#CFLAGS = -O
CFLAGS = -g -Wall -pthread

LD = gcc
LDFLAGS = 

RM = /bin/rm -f

PROG = gridapp

#############################################
all: $(PROG)

$(PROG): gridapp.c
	$(CC) -o $(PROG) gridapp.c $(CFLAGS)

clean:
	$(RM) -rf $(PROG) gridapp.dSYM *.o core *~ *.out







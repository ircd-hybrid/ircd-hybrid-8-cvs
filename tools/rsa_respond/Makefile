# $Id: Makefile,v 1.1 2002/01/04 09:15:05 a1kmm Exp $

all: respond

respond: respond.c
	gcc -g -O2 -Wall -o respond respond.c -lcrypto -ggdb

clean:
	rm -f respond *.core *~

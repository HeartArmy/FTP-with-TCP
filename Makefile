

# Parameters for Makefile operation

CC = gcc
CFLAGS = -Wall -g

all: serverFTP clientFTP 

# serverFTP
serverFTP: serverFTP.o serverHeader.o
	$(CC) $(CFLAGS) -o serverFTP serverFTP.o serverHeader.o -lpthread

serverFTP.o: serverFTP.c serverHeader.h
	$(CC) $(CFLAGS) -c serverFTP.c 

serverheader.o: serverHeader.c serverHeader.h
	$(CC) $(CFLAGS) -c serverHeader.c -lpthread

#clientFTP
clientFTP: clientFTP.o clientHeader.o
	$(CC) $(CFLAGS) -o clientFTP clientFTP.o clientHeader.o 

clientFTP.o: clientFTP.c clientHeader.h
	$(CC) $(CFLAGS) -c clientFTP.c 

clientHeader.o: clientHeader.c clientHeader.h
	$(CC) $(CFLAGS) -c clientHeader.c 

clean: 
	rm -f clientFTP serverFTP serverFTP.o clientFTP.o serverHeader.o clientHeader.o

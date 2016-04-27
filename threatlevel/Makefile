OUTFILE	 = tl
LIBS = -lcurl -lm -lxml2
IFLAGS = -I/usr/include/libxml2
INFILES = $(wildcard *.c)
CC = gcc

install:
	$(CC) $(INFILES) $(IFLAGS) $(LIBS) -o $(OUTFILE)

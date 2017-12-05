CC = gcc
OUTFILE	 = woc 
INFILE = woc.c
LIBS = -lcurl -lm -lxml2
IFLAGS = -I/usr/include/libxml2

ETCDIR = /etc/woc/
DBFILE = wocdb
MAN1DIR = /usr/local/share/man/man1/
MANSRCDIR = ./docs/
MANFILE = woc.1

install-man: 
	mkdir -p $(MAN1DIR)
	cp $(MANSRCDIR)$(MANFILE) $(MAN1DIR)$(MANFILE) 
	gzip -f $(MAN1DIR)$(MANFILE)
	mandb -q

install-bin:
	$(CC) $(INFILE) $(IFLAGS) $(LIBS) -o $(OUTFILE)
	cp $(OUTFILE) /usr/bin/$(OUTFILE)
	mkdir -p $(ETCDIR)
	cp ./$(DBFILE) $(ETCDIR)$(DBFILE) 
	chmod -R a+r $(ETCDIR)

install: install-bin install-man

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

uninstall-woc:
	rm -f $(MAN1DIR)$(MANFILE).gz
	mandb -q
	rm -rf $(ETCDIR)
	rm -f /usr/bin/$(OUTFILE)
	@echo woc uninstalled

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
	@echo woc installed

uninstall: uninstall-woc
install: install-man install-bin

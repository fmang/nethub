DESTDIR=/usr/local
MANDEST=share/man
CFLAGS=-Wall

nethub: nethub.c

doc: nethub.1
	gzip <nethub.1 >nethub.1.gz

install: nethub doc
	mkdir -p $(DESTDIR)/bin $(DESTDIR)/$(MANDEST)/man1
	install -m 755 nethub $(DESTDIR)/bin/
	install -m 644 nethub.1.gz $(DESTDIR)/$(MANDEST)/man1/

uninstall:
	rm -f $(DESTDIR)/bin/nethub
	rm -f $(DESTDIR)/$(MANDEST)/man1/nethub.1.gz

clean:
	rm -f nethub nethub.1.gz

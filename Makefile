CC ?= gcc
CPPFLAGS += -D_LARGEFILE64_SOURCE -D_FILE_OFFSET_BITS=64
DESTDIR ?= /

all: mx-ver.o
	$(CC) \
		$(CFLAGS) $(CPPFLAGS) $(LDFLAGS) \
		-o mx-ver \
		mx-ver.c

%.o: %.c
	$(CC) -c \
		$(CFLAGS) $(CPPFLAGS) $(LDFLAGS) \
		-o $@ \
		$<

install:
	/usr/bin/install -d $(DESTDIR)/bin
	/usr/bin/install mx-ver $(DESTDIR)/bin/mx-ver

clean:
	rm -f *.o mx-ver

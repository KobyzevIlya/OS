CC=gcc
CFLAGS=

all: beauty admirer

beauty: beauty.c common.h
	$(CC) $(CFLAGS) beauty.c common.h -o beauty

admirer: admirer.c common.h
	$(CC) $(CFLAGS) admirer.c common.h -o admirer

clear:
	rm -f beauty admirer

CC=gcc
CFLAGS=

all: beauty admirer father

beauty: beauty.c common.h
	$(CC) $(CFLAGS) beauty.c common.h -o beauty

admirer: admirer.c common.h
	$(CC) $(CFLAGS) admirer.c common.h -o admirer
father: father.c common.h
	$(CC) $(CFLAGS) father.c common.h -o father

clear:
	rm -f beauty admirer father

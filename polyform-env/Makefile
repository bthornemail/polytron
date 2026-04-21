CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lm -lpng

all: test

test: test.c include/polyform.h
	$(CC) $(CFLAGS) -o $@ test.c $(LDFLAGS)

run: test
	./test

clean:
	rm -f test *.png

.PHONY: all run clean
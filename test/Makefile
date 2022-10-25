CC = gcc
CFLAGS = -g -Wall -Wextra
CFLAGS += -I../include

all: nothing.x

%.x: %.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o
	-rm -f *.x
	-rm -fr *.dSYM
	-rm -f *.s


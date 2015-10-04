CC=gcc
CFLAGS=-static -std=c99
OBJS=hash_table.o

all:	$(OBJS)

%.o:	%.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o


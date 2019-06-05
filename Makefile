CC=gcc
CFLAGS=-std=gnu99

all: ringmaster player

ringmaster: ringmaster.c potato.h
	$(CC) $(CFLAGS) -o $@ ringmaster.c

player: player.c potato.h
	$(CC) $(CFLAGS) -o $@ player.c

clean:
	rm -f *~ *.o ringmaster player

clobber:
	rm -f *~ *.o

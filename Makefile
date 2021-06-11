CC=gcc
CFLAGS=-g3 -Wall -lm
commands_exe=$(patsubst commands/%.c,commands/bin/%,$(wildcard commands/*.c))

all: dirBin tsh $(commands_exe)

dirBin:
	mkdir -p commands/bin
	mkdir -p libs/bin

tsh: libs/tarm.c libs/bin/utils.o libs/bin/tar.o libs/bin/copy_move.o
	$(CC) -o tsh libs/bin/utils.o libs/bin/tar.o libs/tarm.c $(CFLAGS)

libs/bin/utils.o: libs/utils.c libs/utils.h
	$(CC) -o libs/bin/utils.o -c libs/utils.c $(CFLAGS)

libs/bin/tar.o: libs/tar.c libs/tar.h
	$(CC) -o libs/bin/tar.o -c libs/tar.c $(CFLAGS)

libs/bin/copy_move.o: libs/copy_move.c libs/copy_move.h
	$(CC) -o libs/bin/copy_move.o -c libs/copy_move.c $(CFLAGS)

$(commands_exe): commands/bin/%: commands/%.c libs/bin/utils.o
	$(CC) -o $@ libs/bin/utils.o libs/bin/tar.o libs/bin/copy_move.o $< $(CFLAGS)

clean:
	rm tsh
	rm libs/bin/*
	rm commands/bin/*
	rmdir libs/bin
	rmdir commands/bin

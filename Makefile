CC = gcc
CFLAGS = -Wall -Wextra -ansi -pedantic

lbp: libpgm/libpgm.o main.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

libpgm/%.o: libpgm/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

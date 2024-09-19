CC = gcc
CFLAGS = -Wall -Wextra -ansi -pedantic

lbp: liblbp/liblbp.o main.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

liblbp/%.o: liblbp/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

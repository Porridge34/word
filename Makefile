CC = gcc
CFLAGS = -g -std=c99 -Wall -Wvla -fsanitize=address,undefined -lm 

all: word

word: word.o
	$(CC) $(CFLAGS) word.o -o word
word.o: word.c
	$(CC) $(CFLAGS) -c word.c

clean:
	rm *.o word
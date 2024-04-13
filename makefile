CC = gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean

all: lexer q writer text2wav

lexer : lexer.c
	$(CC) $(CFLAGS) -o lexer lexer.c

q: parser.c fsm.c queue.c
	$(CC) $(CFLAGS) parser.c fsm.c queue.c -o q 

writer: writer.c
	$(CC) $(CFLAGS) writer.c -o writer -lm

text2wav: text2wav.c
	$(CC) $(CFLAGS) text2wav.c -o text2wav

clean:
	rm -f lexer q writer text2wav


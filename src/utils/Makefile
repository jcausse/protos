CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE -I ../lib/ -g
UTILS := args.o selector.o sockets.o parser.o vrfy.o

.PHONY: all clean

all: $(UTILS)

args.o: args.c args.h
	$(CC) $(CFLAGS) -c args.c -o args.o

selector.o: selector.c selector.h
	$(CC) $(CFLAGS) -c selector.c -o selector.o

sockets.o: sockets.c sockets.h
	$(CC) $(CFLAGS) -c sockets.c -o sockets.o

parser.o: parser.c parser.h
	$(CC) $(CFLAGS) -c parser.c -o parser.o

vrfy.o: vrfy.c vrfy.h
	$(CC) $(CFLAGS) -c vrfy.c -o vrfy.o

clean:
	- rm -f *.o *.gch

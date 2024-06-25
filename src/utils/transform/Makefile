CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -I ../lib/ -g
UTILS := slave.exe central.o

.PHONY: all clean

all: $(UTILS)

slave.exe: slave.c
	$(CC) $(CFLAGS) -c slave.c -o slave.exe

central.o: central.c central.h
	$(CC) $(CFLAGS) -c central.c -o central.o

clean:
	- rm -f *.o *.gch *.exe

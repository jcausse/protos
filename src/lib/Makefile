CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -fsanitize=address
LIBS := hashmap.o linkedlist.o logger.o

.PHONY: all clean

all: $(LIBS)

hashmap.o: hashmap.c hashmap.h
	$(CC) $(CFLAGS) -c hashmap.c -o hashmap.o

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) -c linkedlist.c -o linkedlist.o

logger.o: logger.c logger.h
	$(CC) $(CFLAGS) -c logger.c -o logger.o

clean:
	- rm -f *.o *.gch

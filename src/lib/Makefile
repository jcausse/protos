CFLAGS:= -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L
LIBS:= hashmap linkedlist logger

all: clean $(LIBS)

%: %.c %.h
	$(CC) $(CFLAGS) $^ -c $@.o

clean:
	- rm -f *.o *.gch

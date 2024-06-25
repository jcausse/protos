CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -g
UTILS := args.o
EXEC_NAME := manager.bin

.PHONY: all clean

all: $(UTILS) $(EXEC_NAME)

args.o: args.c args.h
	$(CC) $(CFLAGS) -c args.c -o args.o

$(EXEC_NAME): manager.c manager.h
	$(CC) $(CFLAGS) manager.c -o $(EXEC_NAME)

clean:
	- rm -f $(EXEC_NAME) *.o
CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -g
EXEC_NAME := manager.bin

.PHONY: all clean

all: $(EXEC_NAME)

$(EXEC_NAME): manager.c manager.h
	$(CC) $(CFLAGS) manager.c -o $(EXEC_NAME)

clean:
	- rm -f $(EXEC_NAME)
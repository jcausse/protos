CFLAGS:= -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L
LIBS:= hashmap

all: clean $(LIBS)

%: %.c %_test.c %.h
	$(info *************************************************************)
	$(info ***** Compiling and Testing $@ using Valgrind)
	$(info *************************************************************)
	$(CC) $(CFLAGS) $^ -o $@.bin
	valgrind ./$@.bin

clean:
	- rm -f *.bin

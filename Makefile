CC:=gcc
CFLAGS:= -Wall -Wextra -Werror -std=c11 -fsanitize=address
SRC:= ../src
EXEC:= tpe


all:
	mkdir -p build
	cd build
	$(CC) $(CFLAGS) -c $(SRC)/*.c
	$(CC) $(CFLAGS) *.o -o $(EXEC)



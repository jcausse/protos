#########
# FLAGS #
#########
# -Wall 						: Enable all warnings
# -Wextra 						: Extra warnings
# -Werror 						: Treat all warnings as errors
# -g							: Add debugging symbols
# -fsanitize=address			: Address sanitizer (Google libASan)
# -std=c11						: Use C11
# -D_POSIX_C_SOURCE=200112L 	: Posix version

CFLAGS := -std=c11 -pedantic -pedantic-errors -Wall -Werror -Wextra -D_POSIX_C_SOURCE=200112L -I ./lib -I ./utils -D __USE_DEBUG_LOGS__ -g

SRC_OBJS := main.o sock_types_handlers.o
LIB_OBJS := lib/hashmap.o lib/linkedlist.o lib/logger.o
UTILS_OBJS := utils/args.o utils/selector.o utils/sockets.o utils/parser.o utils/vrfy.o utils/stats.o utils/manager_parser.o

EXEC_NAME := smtpd.bin

.PHONY: all clean

all: smtpd utils/central.exe utils/slave.exe

### LINKER

smtpd: $(SRC_OBJS) $(UTILS_OBJS) $(LIB_OBJS)
	$(CC) $(CFLAGS) $(SRC_OBJS) $(UTILS_OBJS) $(LIB_OBJS) -o $(EXEC_NAME)

### MAIN SOURCE

main.o: main.c messages.h
	$(CC) $(CFLAGS) -c main.c -o main.o

### HANDLERS

sock_types_handlers.o: sock_types_handlers.c sock_types_handlers.h messages.h
	$(CC) $(CFLAGS) -c sock_types_handlers.c -o sock_types_handlers.o

### LIBRARIES

lib/hashmap.o:
	$(MAKE) -C lib hashmap.o

lib/linkedlist.o:
	$(MAKE) -C lib linkedlist.o

lib/logger.o:
	$(MAKE) -C lib logger.o

### UTILITIES

utils/args.o:
	$(MAKE) -C utils args.o

utils/selector.o:
	$(MAKE) -C utils selector.o

utils/sockets.o:
	$(MAKE) -C utils sockets.o

utils/parser.o:
	$(MAKE) -C utils parser.o

utils/vrfy.o:
	$(MAKE) -C utils vrfy.o

utils/stats.o:
	$(MAKE) -C utils stats.o

utils/manager_parser.o:
	$(MAKE) -C utils manager_parser.o

### EXECUTABLES

utils/central.exe:
	$(MAKE) -C utils central.exe

utils/slave.exe:
	$(MAKE) -C utils slave.exe

### OTHER TARGETS

clean:
	- rm -f $(EXEC_NAME) $(SRC_OBJS)
	- $(MAKE) -C lib clean
	- $(MAKE) -C utils clean

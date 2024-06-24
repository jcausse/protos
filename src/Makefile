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
LIBS := lib/hashmap.o lib/linkedlist.o lib/logger.o
UTILS := utils/args.o utils/selector.o utils/sockets.o utils/parser.o utils/vrfy.o utils/transform/transform.o
SRC_OBJS := main.o sock_types_handlers.o

.PHONY: all clean

all: smtpd

smtpd: $(SRC_OBJS) $(UTILS) $(LIBS)
	$(CC) $(CFLAGS) $(SRC_OBJS) $(UTILS) $(LIBS) -o smtpd

main.o: main.c messages.h
	$(CC) $(CFLAGS) -c main.c -o main.o

sock_types_handlers.o: sock_types_handlers.c sock_types_handlers.h
	$(CC) $(CFLAGS) -c sock_types_handlers.c -o sock_types_handlers.o

lib/hashmap.o: 
	$(MAKE) -C lib hashmap.o

lib/linkedlist.o: 
	$(MAKE) -C lib linkedlist.o

lib/logger.o: 
	$(MAKE) -C lib logger.o

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

clean:
	- rm -f smtpd $(SRC_OBJS)
	- $(MAKE) -C lib clean
	- $(MAKE) -C utils clean

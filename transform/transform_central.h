#include "transform_creator.h"
#define MAX_MAILS 100
#define TKN "-"
#define NO_ACTIVE_SLAVE -2
#define TO_TRANSFORM "../auxM"
#define INBOX "inbox"
#include <signal.h>

typedef struct{
    char* buffer;
}Message;

void tControl(const char* command);

Message queue_message(char* buffer);



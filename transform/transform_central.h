#include "transform_creator.h"
#define MAX_MAILS 100
#define TKN "-"
#define NO_ACTIVE_SLAVE -2
#define TO_TRANSFORM "../auxM"
#define INBOX "../inbox"
#include <signal.h>
#define SUCCESS "254 \n"
#define FAILURE "255 \n"

typedef struct{
    char* buffer;
}Message;

void tControl(char* command);

Message queue_message(char* buffer);

static void check_dir(const char * dir){
    struct stat st = {0};
    if (stat(dir, &st) == -1) {
        mkdir(dir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH );
    }
}



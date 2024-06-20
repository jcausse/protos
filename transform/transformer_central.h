#include "transformer_creator.h"
#define MAX_MAILS 100
#define tkn ":"
typedef struct{
    SlaveInfo slave;
    int cID;
}TransformerInfo;

typedef struct{
    char* buffer;
    int cID;
}Message;

void transformer(const char* command);

Message queue_message(char* buffer,int cID);



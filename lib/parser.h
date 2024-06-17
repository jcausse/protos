#ifndef _PARSER_H_
#define _PARSER_H_

/**
 * Basic command enum, it will be used by the client to know how
 * process inner information
 */
typedef enum Command {
    HELO,
    EHLO,
    AUTH,
    MAIL_FROM,
    RCPT_TO,
    DATA,
    RSET,
    QUIT,
    NOOP,
    ERROR
} Command;

typedef struct CommandStructure {
    Command cmd;
    union {
        char * ehloDomain;
        char * authStr;
        char * mailFromStr;
        char * rcptToStr;
        char * dataStr;
        char * errorMsg;
    };
} CommandStructure;

typedef struct StateMachine * StateMachinePtr;
/**
 * Basic structure of the parser, needs to be instantiated,
 * after the parser modifies its state, the command structure
 * will change, so the structure field MUST me processed before parsing
 * another command. It is suggested to copy that information with a
 * function such as strdup or anything that will garantee that
 * the string will not be lost.
 */
typedef struct Parser {
    StateMachinePtr machine;
    char * status;
    CommandStructure * structure;
} Parser;

/**
 * Allocates memory for the parser
 */
Parser * initParser(void);

/**
 * Parses a given string, ended in <CLRF> it will change
 * the inner state of the parser. It should be used one
 * at a time with every given command and inmediatly check
 * what status the parser has.
 *
 * Based on the actual state of the parser, it will decide
 * how to process the given string and it will update the
 * parser structure to contain useful info for the server
 * and a status code if the server needs to reply to the
 * client.
 */
void parseCmd(Parser * parser, const char * command);

/**
 * Destroys the parser, this should be done iif the server is
 * about to finish de service with the client, because all the
 * inner parser state will be lost.
 */
void destroyParser(Parser * parser);

#endif

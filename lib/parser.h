#ifndef _PARSER_H_
#define _PARSER_H_

#define ERR -1
#define TERMINAL -2
#define SUCCESS 0

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
    ERROR,
} Command;

typedef struct CommandStructure {
    Command cmd;
    union {
        char * ehloDomain;
        char * heloDomain;
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
    char * serverDom;
} Parser;

/**
 * Initiates the Regex variables, needs to be called only once by the server
 * the first time it inits to set all the regex used in the server to parse
 * all the arguments given by the client.
 *
 * Returns:
 *
 * SUCCESS if every regex was compiled successfully
 * ERR if any regex have had any error compiling
 *
 * Note: This return value will be useful for debugging if it is any new
 * addition of possible arguments that can be sent by the client.
 */
int compileRegexes(void);

/**
 * Allocates memory for the parser
 */
Parser * initParser(const char * serverDomain);

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
 *
 * Return values are:
 * ERR:      Error parsing a command, expect to have a return message
 *           on the status field
 *
 * SUCCESS:  Parse successful, should check the command structure field
 *           to see what data was extracted from the command, and return
 *           to the client the status string (if any, with the DATA
 *           content the client should not receive any response until
 *           it finishes its input with <CLRF>.<CLRF>).
 *
 * TERMINAL: The parser has reached a terminal status, should use this
 *           value to know when to close the connection with the client
 */
int parseCmd(Parser * parser, const char * command);

/**
 * Destroys the parser, this should be done iif the server is
 * about to finish de service with the client, because all the
 * inner parser state will be lost.
 */
void destroyParser(Parser * parser);

#endif

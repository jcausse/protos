#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <regex.h>

#include "parser.h"
#include "vrfy.h"

#define WELCOME_MSG "Welcome to the SMTP Server!\r\n"
#define HELO_GREETING_MSG "250-%s Hello %s\r\n"
#define EHLO_GREETING_MSG "250-%s Hello %s\n250-TRFM - Triggers email transformation\r\n"

#define SYNTAX_ERROR_MSG "500 Syntax error\r\n"
#define PARAM_SYNTAX_ERROR_MSG "501 Syntax error in parameters or arguments\r\n"
#define CMD_NOT_IMPLEMENTED_MSG "502  Command not implemented\r\n"
#define VRFY_NOT_FOUND "553-Requested action not taken: mailbox name not allowed"
#define VRFY_AMBIGUOUS_MSG "553-Ambiguous; Possibilities are:\n%s\r\n"
#define VRFY_OK_MSG "250-<%s>\r\n"
#define NEED_MAIL_FROM "503-5.5.1 Bad Sequence of Commands. Need MAIL FROM\r\n"
#define NEED_RCPT_TO "503-5.5.1 Bad Sequence of Commands. Need RCPT\r\n"
#define ALREADY_SIGNED "503-5.5.1 Bad Sequence of Commands. You are already identified\r\n"
#define ENTER_DATA_MSG "354 Start mail input; end with <CLRF>.<CLRF>\r\n"
#define QUEUED_MSG "250 Ok. Queued\r\n"
#define QUIT_MSG "221 %s Service closing transmission channel\r\n"
#define GENERIC_OK_MSG "250 OK\r\n"

#define IPV4_REGEX "(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}"
#define IPV6_REGEX "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"
#define DOMAIN_REGEX "/^(([a-zA-Z]{1})|([a-zA-Z]{1}[a-zA-Z]{1})|([a-zA-Z]{1}[0-9]{1})|([0-9]{1}[a-zA-Z]{1})|([a-zA-Z0-9][a-zA-Z0-9-_]{1,61}[a-zA-Z0-9]))\\.([a-zA-Z]{2,6}|[a-zA-Z0-9-]{2,30}\\.[a-zA-Z]{2,3})$/"

/**
 * This is for parsing arguments given by the client, such as
 * the client domain, the auth key, the data message, etc.
 * Parsing specific argument values should be done in a
 * separate function inside the state machine
 */
#define ANY_MSG ""

#define HELO_CMD "HELO"
#define EHLO_CMD "EHLO"
#define VRFY_CMD "VRFY"
#define EXPN_CMD "EXPN"
#define MAIL_CMD "MAIL"
#define RCPT_CMD "RCPT"

#define FROM_ARG " FROM: <"
#define TO_ARG " TO: <"

#define DATA_CMD "DATA"
#define END_DATA "."
#define RSET_CMD "RSET"
#define NOOP_CMD "NOOP"
#define QUIT_CMD "QUIT"
#define TRFM_CMD "TRFM"
#define CMD_LEN 4

#define STATE_CMD_ARR_SIZE 8
#define NEXT_STATE_ARR_SIZE STATE_CMD_ARR_SIZE

#define NO_FLAGS 0
#define SPACE 0x20
#define CLRF_LEN 2

char *strdup(const char *s);
/**
 * States available in the server, this will affect how the parser
 * will behave given the input when it's called.
 */
typedef enum States {
    WELCOME,

    VRFY_PARAM,

    HELO_DOMAIN,
    EHLO_DOMAIN,

    GREETING,

    MAIL_FROM_INPUT,
    MAIL_FROM_OK,

    RCPT_TO_INPUT,
    RCPT_TO_OK,

    DATA_OK,
    END_DATA_OK,

    QUIT_ST
} States;

// Regexes for argument validation
static regex_t ipv4Regex;
static regex_t ipv6Regex;
static regex_t domainRegex;

// State transition functions
static int welcomeTransition(Parser * parser, const char * command);
static int welcomeHeloDomainTransition(Parser * parser, const char * command);
static int welcomeEhloDomainTransition(Parser * parser, const char * command);
static int greetingTransition(Parser * parser, const char * command);
static int mailFromTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);
static int vrfyTransition(Parser * parser, const char * command);

// Auxiliary function to free the command structure
static void freeStruct(Parser * parser);

/**
 * Default state machine that the server receives, it should
 * be modified only by this library, to maintain a correct
 * behaviour.
 */
struct StateMachine {
    enum States currentState;
    enum States priorState;
    enum Command loginState;
};

static int welcomeTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);

    if(strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = HELO_DOMAIN;
        const char * helloDomain = command + CMD_LEN + 1;
        return welcomeHeloDomainTransition(parser, helloDomain);
    }
    else if(strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == SPACE){
        parser->machine->currentState = EHLO_DOMAIN;
        const char * ehloDomain = command + CMD_LEN + 1;
        return welcomeEhloDomainTransition(parser, ehloDomain);
    }
    else if(strncmp(command, RSET_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if(strncmp(command, VRFY_CMD, CMD_LEN) && command[CMD_LEN] == ' ') {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        const char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if(strncmp(command, NOOP_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if(strncmp(command, QUIT_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = QUIT_ST;
        parser->status = strdup(QUIT_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = QUIT;
        return TERMINAL;
    }

    parser->machine->currentState = WELCOME;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = ERROR;
    return ERR;
}

static int welcomeHeloDomainTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char * parsedCmd = (char *) calloc(0, sizeof(char)*len + 1);
    parsedCmd = strncpy(parsedCmd, command, len - 1);

    if(!regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)){
        free(parsedCmd);
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    char * greetingMsg = (char *) calloc(0, sizeof(char)*(strlen(HELO_GREETING_MSG) + strlen(parser->serverDom) + len + 1));
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        free(parsedCmd);
        free(greetingMsg);
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = GREETING;
    parser->machine->loginState = HELO;
    parser->status = greetingMsg;
    if(parser->structure != NULL) freeStruct(parser);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = HELO;
    parser->structure->heloDomain = parsedCmd;
    return SUCCESS;
}

static int welcomeEhloDomainTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char * parsedCmd = (char *) calloc(0, sizeof(char)*len);
    parsedCmd = strncpy(parsedCmd, command, len - 1);

    if(!regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)
        && !regexec(&ipv4Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)
        && !regexec(&ipv6Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)){
        free(parsedCmd);
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    char * greetingMsg = (char *) calloc(0, sizeof(char)*(strlen(EHLO_GREETING_MSG) + strlen(parser->serverDom) + len + 1));
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        free(parsedCmd);
        free(greetingMsg);
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = GREETING;
    parser->status = greetingMsg;
    parser->machine->loginState = EHLO;
    if(parser->structure != NULL) freeStruct(parser);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = EHLO;
    parser->structure->ehloDomain = parsedCmd;
    return SUCCESS;
}

static int greetingTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);

    if(strncmp(command, MAIL_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        const char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = MAIL_FROM_INPUT;
        return mailFromTransition(parser, mailArgs);
    }
    else if(strncmp(command, RCPT_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        parser->machine->currentState = GREETING;
        parser->status = strdup(NEED_MAIL_FROM);
        if(parser->structure != NULL) freeStruct(parser);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if(strncmp(command, DATA_CMD, CMD_LEN) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = GREETING;
        parser->status = strdup(NEED_RCPT_TO);
        if(parser->structure != NULL) freeStruct(parser);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if(strncmp(command, RSET_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if(strncmp(command, VRFY_CMD, CMD_LEN) && command[CMD_LEN] == ' ') {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        const char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if(strncmp(command, EXPN_CMD, CMD_LEN) && command[CMD_LEN] == ' ') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = EXPN;
        return ERR;
    }
    else if(strncmp(command, TRFM_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        if(parser->machine->loginState == HELO){
            parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
            parser->structure = malloc(sizeof(CommandStructure));
            parser->structure->cmd = TRFM;
            return ERR;
        }
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = TRFM;
        parser->transform = !parser->transform;
    }
    else if(strncmp(command, NOOP_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if(strncmp(command, QUIT_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = QUIT_ST;
        parser->status = strdup(QUIT_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = QUIT;
        return TERMINAL;
    }
    else if((strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')
         || (strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')) {
        parser->machine->currentState = GREETING;
        parser->status = strdup(ALREADY_SIGNED);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = GREETING;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = ERROR;
    return ERR;
}

static int vrfyTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);
    unsigned long len = strlen(command) - CLRF_LEN + 1;
    char * arg = (char *) malloc(sizeof(char)*len);
    char * result;
    int res = vrfy(result,  arg);
    if(res == NONE) {
        parser->status = strdup(VRFY_NOT_FOUND);
        parser->machine->currentState = parser->machine->priorState;
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = VRFY;
        return ERR;
    }
    else if(res == SINGLE) {
        len = strlen(VRFY_OK_MSG) + strlen(result) + 1;
        char * status = (char*) malloc(sizeof(char)*len);
        sprintf(status, VRFY_OK_MSG, result);
        parser->status = status;
        parser->machine->currentState = parser->machine->priorState;
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = VRFY;
        return SUCCESS;
    }

    len = strlen(VRFY_AMBIGUOUS_MSG) + strlen(result) + 1;
    char * status = (char*) malloc(sizeof(char)*len);
    sprintf(status, VRFY_AMBIGUOUS_MSG, result);
    parser->status = status;
    parser->machine->currentState = parser->machine->priorState;
    parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
    parser->structure->cmd = VRFY;
    return SUCCESS;
}

static int mailFromTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);


static void freeStruct(Parser * parser) {
    if(parser->structure == NULL) return;
    switch(parser->structure->cmd){
        case HELO: if(parser->structure->heloDomain != NULL) free(parser->structure->heloDomain); break;
        case EHLO: if(parser->structure->ehloDomain != NULL) free(parser->structure->ehloDomain); break;
        case MAIL_FROM: if(parser->structure->mailFromStr != NULL) free(parser->structure->mailFromStr); break;
        case RCPT_TO: if(parser->structure->rcptToStr != NULL) free(parser->structure->rcptToStr); break;
        case DATA: if(parser->structure->dataStr != NULL) free(parser->structure->dataStr); break;
        default: break;
    }
    free(parser->structure);
}

/**
 * Initiates the Regex variables, needs to be called only once by the server
 * the first time it inits to set all the regex used in the server to parse
 * all the arguments given by the client.
 */
int compileRegexes(void) {
    if(!regcomp(&ipv4Regex, IPV4_REGEX, NO_FLAGS) ||
       !regcomp(&ipv6Regex, IPV6_REGEX, NO_FLAGS) ||
       !regcomp(&domainRegex, DOMAIN_REGEX, NO_FLAGS)) return ERR;
    return SUCCESS;
}

/**
 * Allocates the necessary memory for the State Machine and
 * for the parser.
 */
Parser * initParser(const char * serverDomain) {
    StateMachinePtr sm = malloc(sizeof(struct StateMachine));
    sm->currentState = WELCOME;
    Parser * parser = malloc(sizeof(Parser));
    parser->serverDom = strdup(serverDomain);
    parser->machine = sm;
    parser->status = strdup(WELCOME_MSG);
    parser->structure = NULL;
    parser->transform = false;
    return parser;
}

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
int parseCmd(Parser * parser, const char * command) {
    if(parser == NULL || parser->machine == NULL) return TERMINAL;
    switch(parser->machine->currentState){
        case WELCOME: return welcomeTransition(parser, command);

        case GREETING: return greetingTransition(parser, command);

        case MAIL_FROM_OK: return mailFromTransition(parser, command);

        case RCPT_TO_OK: return rcptToTransition(parser, command);

        case DATA_OK: return dataTransition(parser, command);

        default: return TERMINAL; // Unexpected parsing error, should never get here
    }
}

/**
 * Destroys the parser, this should be done iif the server is
 * about to finish de service with the client, because all the
 * inner parser state will be lost.
 */
void destroy(Parser * parser) {
    if(parser == NULL) return;
    free(parser->machine);
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);
    if(parser->serverDom != NULL) free(parser->serverDom);
    free(parser);
}

#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <regex.h>

#include "parser.h"

#define WELCOME_MSG "Welcome to the SMTP Server!\r\n"
#define HELO_GREETING_MSG "250-%s Hello %s\r\n"
#define EHLO_GREETING_MSG "250-%s Hello %s\r\n"

#define SYNTAX_ERROR_MSG "500 Syntax error\r\n"
#define PARAM_SYNTAX_ERROR_MSG "501 Syntax error in parameters or arguments\r\n"
#define NEED_MAIL_FROM "503-5.5.1 Bad Sequence of Commands. Need MAIL FROM\r\n"
#define NEED_RCPT_TO "503-5.5.1 Bad Sequence of Commands. Need RCPT\r\n"
#define ALREADY_SIGNED "503-5.5.1 Bad Sequence of Commands. You are already identified\r\n"
#define ENTER_DATA_MSG "354 Start mail input; end with <CLRF>.<CLRF>\r\n"
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
#define MAIL_CMD "MAIL"
#define RCPT_CMD "RCPT"

#define FROM_ARG " FROM: <"
#define TO_ARG " TO: <"

#define DATA_CMD "DATA"
#define END_DATA "."
#define RSET_CMD "RSET"
#define NOOP_CMD "NOOP"
#define QUIT_CMD "QUIT"
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
    HELO_DOMAIN,
    EHLO_DOMAIN,
    HELO_GREETING,
    GREETING,
    HELO_MAIL_FROM_INPUT,
    MAIL_FROM_INPUT,
    HELO_MAIL_FROM_OK,
    MAIL_FROM_OK,
    HELO_RCPT_TO_INPUT,
    RCPT_TO_INPUT,
    HELO_RCPT_TO_OK,
    RCPT_TO_OK,
    HELO_DATA_OK,
    DATA_OK,
    HELO_END_DATA_OK,
    END_DATA_OK,
    QUIT_ST
} States;

// Regexes for argument validation
static regex_t ipv4Regex;
static regex_t ipv6Regex;
static regex_t domainRegex;


/**
 * Basic structure to handle every posible state of the service
 * it should be similar to a state machine, with the difference
 * that it will process the entire string instead of a single
 * character out of the box. This is only for simplicity reasons.
 */
struct State {
    States state;
    char * cmdExpected[STATE_CMD_ARR_SIZE];
    char * defaultMsg;
};

// State transition functions
static int welcomeTransition(Parser * parser, const char * command);
static int welcomeHeloDomainTransition(Parser * parser, const char * command);
static int welcomeDomainTransition(Parser * parser, const char * command);
static int heloTransition(Parser * parser, const char * command);
static int ehloTransition(Parser * parser, const char * command);
static int mailFromTransition(Parser * parser, const char * command);
static int heloMailFromTransition(Parser * parser, const char * command);
static int heloRcptToTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);
static int heloDataTransition(Parser * parser, const char * command);
static int quitTransition(Parser * parser, const char * command);

// Auxiliary function to free the command structure
static void freeStruct(Parser * parser);

// POSSIBLE STATES
static struct State quitState = {
    .state = QUIT_ST,
    .cmdExpected = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = QUIT_MSG
};

static struct State dataState = {
    .state       = DATA_OK,
    .cmdExpected = { END_DATA, ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = ENTER_DATA_MSG
};

static struct State rcptSuccessState = {
    .state       = RCPT_TO_OK,
    .cmdExpected = { DATA_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, MAIL_CMD, RCPT_CMD, HELO_CMD, EHLO_CMD},
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State rcptToState = {
    .state       = RCPT_TO_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL,
};

static struct State mailSuccessState = {
    .state       = MAIL_FROM_OK,
    .cmdExpected = { RCPT_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, DATA_CMD, MAIL_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State mailFromState = {
    .state       = MAIL_FROM_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State ehloState = {
    .state       = GREETING,
    .cmdExpected = { MAIL_CMD, RCPT_CMD, DATA_CMD, QUIT_CMD, NOOP_CMD, RSET_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = EHLO_GREETING_MSG,
};

static struct State heloDataState = {
    .state       = DATA_OK,
    .cmdExpected = { END_DATA, ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = ENTER_DATA_MSG
};

static struct State heloRcptSuccessState = {
    .state       = HELO_RCPT_TO_OK,
    .cmdExpected = { DATA_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, MAIL_CMD, RCPT_CMD, HELO_CMD, EHLO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State heloRcptToState = {
    .state       = HELO_RCPT_TO_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL,
};

static struct State heloMailSuccessState = {
    .state       = HELO_MAIL_FROM_OK,
    .cmdExpected = { RCPT_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, DATA_CMD, MAIL_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State heloMailFromState = {
    .state       = HELO_MAIL_FROM_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State heloState = {
    .state       = HELO_GREETING,
    .cmdExpected = { MAIL_CMD, QUIT_CMD, NOOP_CMD, RSET_CMD, RCPT_CMD, DATA_CMD, HELO_CMD, EHLO_CMD },
    .defaultMsg  = HELO_GREETING_MSG
};

static struct State welcomeHeloDomainState = {
    .state       = HELO_DOMAIN,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State welcomeEhloDomainState = {
    .state       = EHLO_DOMAIN,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State welcomeState = {
    .state       = WELCOME,
    .cmdExpected = { HELO_CMD, EHLO_CMD, QUIT_CMD, NOOP_CMD, RSET_CMD, NULL, NULL, NULL },
    .defaultMsg  = WELCOME_MSG,
};


/**
 * Default state machine that the server receives, it should
 * be modified only by this library, to maintain a correct
 * behaviour.
 */
struct StateMachine {
    struct State * currentState;
    struct State * initialState;
};

static int welcomeTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);

    if(strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = &welcomeHeloDomainState;
        const char * helloDomain = command + CMD_LEN + 1;
        return welcomeHeloDomainTransition(parser, helloDomain);
    }
    else if(strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == SPACE){
        parser->machine->currentState = &welcomeEhloDomainState;
        const char * ehloDomain = command + CMD_LEN + 1;
        return welcomeDomainTransition(parser, ehloDomain);
    }
    else if(strncmp(command, RSET_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, NOOP_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, QUIT_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &quitState;
        parser->status = strdup(QUIT_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return TERMINAL;
    }

    parser->machine->currentState = &welcomeState;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    if(parser->structure != NULL) freeStruct(parser);
    return ERR;
}

static int welcomeHeloDomainTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char * parsedCmd = (char *) calloc(0, sizeof(char)*len + 1);
    parsedCmd = strncpy(parsedCmd, command, len - 1);

    if(!regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)){
        free(parsedCmd);
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    char * greetingMsg = (char *) calloc(0, sizeof(char)*(strlen(HELO_GREETING_MSG) + strlen(parser->serverDom) + len + 1));
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        free(parsedCmd);
        free(greetingMsg);
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    parser->machine->currentState = &heloState;
    parser->status = greetingMsg;
    if(parser->structure != NULL) freeStruct(parser);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = HELO;
    parser->structure->heloDomain = parsedCmd;
    return SUCCESS;
}

static int welcomeDomainTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char * parsedCmd = (char *) calloc(0, sizeof(char)*len);
    parsedCmd = strncpy(parsedCmd, command, len - 1);

    if(!regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)
        && !regexec(&ipv4Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)
        && !regexec(&ipv6Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)){
        free(parsedCmd);
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    char * greetingMsg = (char *) calloc(0, sizeof(char)*(strlen(EHLO_GREETING_MSG) + strlen(parser->serverDom) + len + 1));
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        free(parsedCmd);
        free(greetingMsg);
        parser->machine->currentState = &welcomeState;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        return ERR;
    }

    parser->machine->currentState = &ehloState;
    parser->status = greetingMsg;
    if(parser->structure != NULL) freeStruct(parser);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = EHLO;
    parser->structure->ehloDomain = parsedCmd;
    return SUCCESS;
}

static int heloTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);

    if(strncmp(command, MAIL_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        const char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = &heloMailFromState;
        return heloMailFromTransition(parser, mailArgs);
    }
    else if(strncmp(command, RCPT_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        parser->machine->currentState = &heloState;
        parser->status = strdup(NEED_MAIL_FROM);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }
    else if(strncmp(command, DATA_CMD, CMD_LEN) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = &heloState;
        parser->status = strdup(NEED_RCPT_TO);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }
    else if(strncmp(command, RSET_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &heloState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, NOOP_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &heloState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, QUIT_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &quitState;
        parser->status = strdup(QUIT_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return TERMINAL;
    }
    else if((strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')
         || (strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')) {
        parser->machine->currentState = &heloState;
        parser->status = strdup(ALREADY_SIGNED);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }

    parser->machine->currentState = &heloState;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    if(parser->structure != NULL) freeStruct(parser);
    return ERR;
}

static int ehloTransition(Parser * parser, const char * command) {
    if(parser->status != NULL) free(parser->status);

    if(strncmp(command, MAIL_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        const char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = &mailFromState;
        return mailFromTransition(parser, mailArgs);
    }
    else if(strncmp(command, RCPT_CMD, CMD_LEN) && command[CMD_LEN] == ' '){
        parser->machine->currentState = &ehloState;
        parser->status = strdup(NEED_MAIL_FROM);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }
    else if(strncmp(command, DATA_CMD, CMD_LEN) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = &ehloState;
        parser->status = strdup(NEED_RCPT_TO);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }
    else if(strncmp(command, RSET_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &ehloState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, NOOP_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &ehloState;
        parser->status = strdup(GENERIC_OK_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return SUCCESS;
    }
    else if(strncmp(command, QUIT_CMD, CMD_LEN) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = &quitState;
        parser->status = strdup(QUIT_MSG);
        if(parser->structure != NULL) freeStruct(parser);
        return TERMINAL;
    }
    else if((strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')
         || (strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')) {
        parser->machine->currentState = &ehloState;
        parser->status = strdup(ALREADY_SIGNED);
        if(parser->structure != NULL) freeStruct(parser);
        return ERR;
    }

    parser->machine->currentState = &ehloState;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    if(parser->structure != NULL) freeStruct(parser);
    return ERR;
}

static int mailFromTransition(Parser * parser, const char * command);
static int heloMailFromTransition(Parser * parser, const char * command);
static int heloRcptToTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);
static int heloDataTransition(Parser * parser, const char * command);
static int quitTransition(Parser * parser, const char * command);


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
    sm->currentState = &welcomeState;
    sm->initialState = &welcomeState;
    Parser * parser = malloc(sizeof(Parser));
    parser->serverDom = strdup(serverDomain);
    parser->machine = sm;
    parser->status = strdup(WELCOME_MSG);
    parser->structure = NULL;
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
    switch(parser->machine->currentState->state){
        case WELCOME: return welcomeTransition(parser, command);

        case HELO_GREETING: return heloTransition(parser, command);
        case GREETING: return ehloTransition(parser, command);

        case HELO_MAIL_FROM_OK: return heloMailFromTransition(parser, command);
        case MAIL_FROM_OK: return mailFromTransition(parser, command);

        case HELO_RCPT_TO_OK: return heloRcptToTransition(parser, command);
        case RCPT_TO_OK: return rcptToTransition(parser, command);

        case HELO_DATA_OK: return heloDataTransition(parser, command);
        case DATA_OK: return dataTransition(parser, command);

        case QUIT_ST: return quitTransition(parser, command);
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

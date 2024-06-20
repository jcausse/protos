#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "parser.h"

#define WELCOME_MSG "Welcome to the SMTP Server!"
#define HELO_GREETING_MSG "250-%s Hello %s"
#define EHLO_GREETING_MSG "250-%s Hello %s\n250-AUTH"
#define SYNTAX_ERROR_MSG "500 Syntax error"
#define PARAM_SYNTAX_ERROR_MSG "501 Syntax error in parameters or arguments"
#define NEED_MAIL_FROM "503-5.5.1 Need MAIL FROM"
#define NEED_RCPT_TO "503-5.5.1 Need RCPT"
#define ENTER_DATA_MSG "354 Start mail input; end with <CLRF>.<CLRF>"
#define QUIT_MSG "221 %s Service closing transmission channel"
#define GENERIC_OK_MSG "250 OK"

/**
 * This is for parsing arguments given by the client, such as
 * the client domain, the auth key, the data message, etc.
 * Parsing specific argument values should be done in a
 * separate function inside the state machine
 */
#define ANY_MSG ""

#define HELO_CMD "HELO "
#define EHLO_CMD "EHLO "
#define MAIL_FROM_CMD "MAIL FROM: <"
#define RCPT_TO_CMD "RCPT TO: <"
#define CLOSE_RCPT_MAIL ">"
#define DATA_CMD "DATA"
#define END_DATA_CMD "."
#define RSET_CMD "RSET"
#define NOOP_CMD "NOOP"
#define QUIT_CMD "QUIT"

#define STATE_CMD_ARR_SIZE 8
#define NEXT_STATE_ARR_SIZE STATE_CMD_ARR_SIZE

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
static int heloTransition(Parser * parser, const char * command);
static int ehloTransition(Parser * parser, const char * command);
static int mailFromTransition(Parser * parser, const char * command);
static int heloMailFromTransition(Parser * parser, const char * command);
static int heloRcptToTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);
static int heloDataTransition(Parser * parser, const char * command);
static int quitTransition(Parser * parser, const char * command);


// POSSIBLE STATES
static struct State quitState = {
    .state = QUIT_ST,
    .cmdExpected = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = QUIT_MSG
};

static struct State dataState = {
    .state       = DATA_OK,
    .cmdExpected = { END_DATA_CMD, ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = ENTER_DATA_MSG
};

static struct State rcptSuccessState = {
    .state       = RCPT_TO_OK,
    .cmdExpected = { DATA_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, MAIL_FROM_CMD, RCPT_TO_CMD, HELO_CMD, EHLO_CMD},
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State rcptToState = {
    .state       = RCPT_TO_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL,
};

static struct State mailSuccessState = {
    .state       = MAIL_FROM_OK,
    .cmdExpected = { RCPT_TO_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, DATA_CMD, MAIL_FROM_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State mailFromState = {
    .state       = MAIL_FROM_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State ehloState = {
    .state       = GREETING,
    .cmdExpected = { MAIL_FROM_CMD, RCPT_TO_CMD, DATA_CMD, QUIT_CMD, NOOP_CMD, RSET_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = EHLO_GREETING_MSG,
};

static struct State heloDataState = {
    .state       = DATA_OK,
    .cmdExpected = { END_DATA_CMD, ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = ENTER_DATA_MSG
};

static struct State heloRcptSuccessState = {
    .state       = HELO_RCPT_TO_OK,
    .cmdExpected = { DATA_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, MAIL_FROM_CMD, RCPT_TO_CMD, HELO_CMD, EHLO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State heloRcptToState = {
    .state       = HELO_RCPT_TO_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL,
};

static struct State heloMailSuccessState = {
    .state       = HELO_MAIL_FROM_OK,
    .cmdExpected = { RCPT_TO_CMD, QUIT_CMD, RSET_CMD, NOOP_CMD, DATA_CMD, MAIL_FROM_CMD, EHLO_CMD, HELO_CMD },
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State heloMailFromState = {
    .state       = HELO_MAIL_FROM_INPUT,
    .cmdExpected = { ANY_MSG, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .defaultMsg  = NULL
};

static struct State heloState = {
    .state       = HELO_GREETING,
    .cmdExpected = { MAIL_FROM_CMD, QUIT_CMD, NOOP_CMD, RSET_CMD, RCPT_TO_CMD, DATA_CMD, HELO_CMD, EHLO_CMD },
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
    unsigned long heloCmdLen = strlen(HELO_CMD);
    unsigned long ehloCmdLen = strlen(EHLO_CMD);

    if(strncmp(command, HELO_CMD, heloCmdLen)) {
        parser->machine->currentState = &welcomeHeloDomainState;

    }
}

static int heloTransition(Parser * parser, const char * command);
static int ehloTransition(Parser * parser, const char * command);
static int mailFromTransition(Parser * parser, const char * command);
static int heloMailFromTransition(Parser * parser, const char * command);
static int heloRcptToTransition(Parser * parser, const char * command);
static int rcptToTransition(Parser * parser, const char * command);
static int dataTransition(Parser * parser, const char * command);
static int heloDataTransition(Parser * parser, const char * command);
static int quitTransition(Parser * parser, const char * command);
/**
 * Allocates the necessary memory for the State Machine and
 * for the parser.
 */
Parser * initParser(void) {
    StateMachinePtr sm = malloc(sizeof(struct StateMachine));
    sm->currentState = &welcomeState;
    sm->initialState = &welcomeState;
    Parser * parser = malloc(sizeof(Parser));
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
    if(parser->structure != NULL) free(parser->structure);
    free(parser);
}

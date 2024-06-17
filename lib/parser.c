#include <string.h>
#include <strings.h>
#include <stdlib.h>

#include "parser.h"

#define WELCOME_MSG "Welcome to the SMTP Server!"
#define HELO_GREETING_MSG "250-%s Hello %s"
#define EHLO_GREETING_MSG "250-%s Hello %s\n250-AUTH"
#define SERVICE_NOT_AVAILABLE_MSG "421 %s Service not available, closing transmission channel"
#define NEED_AUTH_MSG "530-5.5.1 Authentication Required"
#define AUTH_OK_MSG "235-2.7.0 Authentication Succeded"
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
#define AUTH_CMD "AUTH PLAIN "
#define MAIL_FROM_CMD "MAIL FROM: <"
#define RCPT_TO_CMD "RCPT TO: <"
#define CLOSE_RCPT_MAIL ">"
#define DATA_CMD "DATA"
#define END_DATA_CMD "."
#define RSET_CMD "RSET"
#define NOOP_CMD "NOOP"
#define QUIT_CMD "QUIT"

#define STATE_CMD_ARR_SIZE 9
#define NEXT_STATE_ARR_SIZE STATE_CMD_ARR_SIZE
#define ERR_STATE_SIZE 1

/**
 * States available in the server, this will affect how the parser
 * will behave given the input when it's called.
 */
typedef enum States {
    WELCOME,
    HELO_DOMAIN,
    EHLO_DOMAIN,
    HELO_GREETING,
    EHLO_GREETING,
    SERVICE_NOT_AVAILABLE, // Should close transmission channel
    NEED_AUTH,
    AUTH_PSSW,
    AUTH_OK,
    MAIL_FROM_INPUT,
    MAIL_FROM_OK,
    RCPT_TO_INPUT,
    RCPT_TO_OK,
    DATA_OK,
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
    struct State * nextState[NEXT_STATE_ARR_SIZE];
    struct State * errState[ERR_STATE_SIZE];
    char * defaultMsg;
};

// POSSIBLE STATES
static struct State quitState = {
    .state       = QUIT_ST,
    .cmdExpected = NULL,
    .nextState   = NULL,
    .errState    = NULL,
    .defaultMsg  = QUIT_MSG
};

static struct State serviceNotAvailableState = {

};

static struct State dataState = {
    .state       = DATA_OK,
    .cmdExpected = { END_DATA_CMD,  ANY_MSG,    NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &ehloState,    &dataState, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = NULL,
    .defaultMsg  = ENTER_DATA_MSG
};

static struct State rcptSuccessState = {
    .state       = RCPT_TO_OK,
    .cmdExpected = { DATA_CMD,      QUIT_CMD,   RSET_CMD,   NOOP_CMD,           MAIL_FROM_CMD,      RCPT_TO_CMD,        HELO_CMD,           EHLO_CMD,           NULL },
    .nextState   = { &dataState,    &quitState, &ehloState, &rcptSuccessState,  &rcptSuccessState,  &rcptSuccessState,  &rcptSuccessState,  &rcptSuccessState,  NULL },
    .errState    = NULL,
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State rcptToState = {
    .state       = RCPT_TO_INPUT,
    .cmdExpected = { ANY_MSG,           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &rcptSuccessState, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = { &mailSuccessState },
    .defaultMsg  = NULL,
};

static struct State mailSuccessState = {
    .state       = MAIL_FROM_OK,
    .cmdExpected = { RCPT_TO_CMD,   QUIT_CMD,   RSET_CMD,   NOOP_CMD,           DATA_CMD,           MAIL_FROM_CMD,      EHLO_CMD,           HELO_CMD,           NULL },
    .nextState   = { &rcptToState,  &quitState, &ehloState, &mailSuccessState,  &mailSuccessState,  &mailSuccessState,  &mailSuccessState,  &mailSuccessState,  NULL },
    .errState    = NULL,
    .defaultMsg  = GENERIC_OK_MSG
};

static struct State mailFromState = {
    .state       = MAIL_FROM_INPUT,
    .cmdExpected = { ANY_MSG,           NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &mailSuccessState, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = { &authSuccessState },
    .defaultMsg  = NULL
};

static struct State authSuccessState = {
    .state       = AUTH_OK,
    .cmdExpected = { MAIL_FROM_CMD,     QUIT_CMD,   NOOP_CMD,           RSET_CMD,   RCPT_TO_CMD,    DATA_CMD,   EHLO_CMD,           HELO_CMD,           NULL },
    .nextState   = { &mailFromState,    &quitState, &authSuccessState,  &ehloState, NULL,           NULL,       &authSuccessState,  &authSuccessState,  NULL },
    .errState    = NULL,
    .defaultMsg  = AUTH_OK_MSG
};

static struct State authState = {
    .state       = AUTH_PSSW,
    .cmdExpected = { ANY_MSG,              NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &authSuccessState,    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = NULL
    .defaultMsg  = NULL
};


static struct State heloState = {
    .state       = HELO_GREETING,
    .cmdExpected = { QUIT_CMD,   NOOP_CMD,   RSET_CMD,   MAIL_FROM_CMD,  RCPT_TO_CMD,    DATA_CMD,   HELO_CMD,      EHLO_CMD,   NULL },
    .nextState   = { &quitState, &heloState, &heloState, &heloState,     &heloState,     &heloState, &heloState,    &heloState, NULL },
    .errState    = { &serviceNotAvailableState },
    .defaultMsg  = HELO_GREETING_MSG
};

static struct State ehloState = {
    .state       = EHLO_GREETING,
    .cmdExpected = { AUTH_CMD,     MAIL_FROM_CMD,  RCPT_TO_CMD,    DATA_CMD,   QUIT_CMD,   NOOP_CMD,   RSET_CMD,    EHLO_CMD,   HELO_CMD},
    .nextState   = { &authState,   &ehloState,     &ehloState,     &ehloState, &quitState, &ehloState, &ehloState,  &ehloState, &ehloState },
    .errState    = NULL,
    .defaultMsg  = EHLO_GREETING_MSG,
};

static struct State welcomeHeloDomainState = {
    .state       = HELO_DOMAIN,
    .cmdExpected = { ANY_MSG,      NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &ehloState,   NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = { &welcomeState },
    .defaultMsg  = NULL
};

static struct State welcomeEhloDomainState = {
    .state       = EHLO_DOMAIN,
    .cmdExpected = { ANY_MSG,      NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .nextState   = { &heloState,   NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    .errState    = NULL,
    .defaultMsg  = NULL
};

static struct State welcomeState = {
    .state       = WELCOME,
    .cmdExpected = { HELO_CMD,                 EHLO_CMD,                   QUIT_CMD,   NOOP_CMD,       RSET_CMD,       NULL, NULL, NULL, NULL },
    .nextState   = { &welcomeHeloDomainState,  &welcomeEhloDomainState,    &quitState, &welcomeState,  &welcomeState,  NULL, NULL, NULL, NULL },
    .errState    = NULL,
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
 */
void parseCmd(Parser * parser, const char * command);

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

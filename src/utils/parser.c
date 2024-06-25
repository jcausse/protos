#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>
#include <regex.h>

#include "parser.h"
#include "vrfy.h"

#define WELCOME_MSG "Welcome to the SMTP Server!\r\n"
#define HELO_GREETING_MSG "250-%s Hello %s\r\n"
#define EHLO_GREETING_MSG "250-%s Hello %s\n250-TRFM - Triggers email transformation\r\n"

#define SYNTAX_ERROR_MSG "500 Syntax error\r\n"
#define PARAM_SYNTAX_ERROR_MSG "501 Syntax error in parameters or arguments\r\n"
#define CMD_NOT_IMPLEMENTED_MSG "502  Command not implemented\r\n"
#define VRFY_NOT_FOUND "553-Failure: Mailbox name not found\r\n"
#define VRFY_AMBIGUOUS_MSG "553-Ambiguous; Possibilities are:\n%s\r\n"
#define VRFY_AMBIGUOUS_INIT_LINE "553-<"
#define VRFY_OK_MSG "250-<%s>\r\n"
#define NEED_MAIL_FROM "503-5.5.1 Bad Sequence of Commands. Need MAIL FROM\r\n"
#define NEED_RCPT_TO "503-5.5.1 Bad Sequence of Commands. Need RCPT\r\n"
#define MAIL_FROM_ALREADY_IN "503-5.5.1 Bad Sequence of Commands. Mail From has been already sent.\r\n"
#define RCPT_TO_ALREADY_IN "503-5.5.1 Bad Sequence of Commands. Rcpt To has been already sent.\r\n"
#define ALREADY_SIGNED "503-5.5.1 Bad Sequence of Commands. You are already identified\r\n"
#define ENTER_DATA_MSG "354 Start mail input; end with <CLRF>.<CLRF>\r\n"
#define QUEUED_MSG "250 Ok. Queued\r\n"
#define QUIT_MSG "221 %s Service closing transmission channel\r\n"
#define GENERIC_OK_MSG "250 OK\r\n"

#define IPV4_REGEX "(\\b25[0-5]|\\b2[0-4][0-9]|\\b[01]?[0-9][0-9]?)(\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)){3}"
#define IPV6_REGEX "(([0-9a-fA-F]{1,4}:){7,7}[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,7}:|([0-9a-fA-F]{1,4}:){1,6}:[0-9a-fA-F]{1,4}|([0-9a-fA-F]{1,4}:){1,5}(:[0-9a-fA-F]{1,4}){1,2}|([0-9a-fA-F]{1,4}:){1,4}(:[0-9a-fA-F]{1,4}){1,3}|([0-9a-fA-F]{1,4}:){1,3}(:[0-9a-fA-F]{1,4}){1,4}|([0-9a-fA-F]{1,4}:){1,2}(:[0-9a-fA-F]{1,4}){1,5}|[0-9a-fA-F]{1,4}:((:[0-9a-fA-F]{1,4}){1,6})|:((:[0-9a-fA-F]{1,4}){1,7}|:)|fe80:(:[0-9a-fA-F]{0,4}){0,4}%[0-9a-zA-Z]{1,}|::(ffff(:0{1,4}){0,1}:){0,1}((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])|([0-9a-fA-F]{1,4}:){1,4}:((25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9])\\.){3,3}(25[0-5]|(2[0-4]|1{0,1}[0-9]){0,1}[0-9]))"
#define DOMAIN_REGEX "^[0-9a-zA-Z]+((\\.[a-zA-Z]{2,})+([.][a-zA-Z]{2,3})?)?$"
#define MAIL_REGEX "^[a-zA-Z0-9]+([._+-][a-zA-Z0-9]+)*@[0-9a-zA-Z]+(\\.[a-zA-Z]{2,})+([.][a-zA-Z]{2,3})?$"

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
#define RSET_CMD "RSET"
#define NOOP_CMD "NOOP"
#define QUIT_CMD "QUIT"
#define TRFM_CMD "TRFM"
#define CMD_LEN 4

#define FROM_ARG "FROM: <"
#define FROM_ARG_LEN strlen(FROM_ARG)

#define TO_ARG "TO: <"
#define TO_ARG_LEN strlen(TO_ARG)

#define DATA_CMD "DATA"
#define END_DATA ".\r\n"
#define END_DATA_LEN strlen(END_DATA)

#define NO_FLAGS 0
#define SPACE ' '
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

    DATA_INPUT,

    QUIT_ST
} States;

// Regexes for argument validation
static regex_t ipv4Regex;
static regex_t ipv6Regex;
static regex_t domainRegex;
static regex_t mailRegex;

// State transition functions
static int welcomeTransition(Parser parser, char * command);
static int welcomeHeloDomainTransition(Parser parser, char * command);
static int welcomeEhloDomainTransition(Parser parser, char * command);
static int greetingTransition(Parser parser, char * command);
static int mailFromTransition(Parser parser, char * command);
static int mailFromOkTransition(Parser parser, char * command);
static int rcptToTransition(Parser parser, char * command);
static int rcptToOkTransition(Parser parser, char * command);
static int dataTransition(Parser parser, char * command);
static int vrfyTransition(Parser parser, char * command);

// Auxiliary function to free the command structure
static void freeStruct(Parser parser);

static void toUpperCmd(char * command);
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

static int welcomeTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);
    toUpperCmd(command);

    if(strncmp(command, HELO_CMD, CMD_LEN) == SUCCESS && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = HELO_DOMAIN;
        char * helloDomain = command + CMD_LEN + 1;
        return welcomeHeloDomainTransition(parser, helloDomain);
    }
    else if(strncmp(command, EHLO_CMD, CMD_LEN) == SUCCESS && command[CMD_LEN] == SPACE){
        parser->machine->currentState = EHLO_DOMAIN;
        char * ehloDomain = command + CMD_LEN + 1;
        return welcomeEhloDomainTransition(parser, ehloDomain);
    }
    else if((strncmp(command, RSET_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if((strncmp(command, VRFY_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == ' ') {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if((strncmp(command, NOOP_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if((strncmp(command, QUIT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
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

static int welcomeHeloDomainTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char parsedCmd[256] = {0};
    strncpy(parsedCmd, command, len - 1);

    if(regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS) == REG_NOMATCH){
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    char greetingMsg [512] = {0};
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = GREETING;
    parser->machine->loginState = HELO;
    parser->status = strdup(greetingMsg);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = HELO;
    parser->structure->heloDomain = strdup(parsedCmd);
    return SUCCESS;
}

static int welcomeEhloDomainTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);

    if(command[0] == '\0' || command[0] == '\r' || command[0] == '\n') {
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    unsigned long len = strlen(command) - CLRF_LEN + 1; // Size of the argument plus null
    char parsedCmd[256] = {0};
    strncpy(parsedCmd, command, len - 1);

    if( regexec(&domainRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS) == REG_NOMATCH
     && regexec(&ipv4Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)   == REG_NOMATCH
     && regexec(&ipv6Regex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS)   == REG_NOMATCH){
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    char greetingMsg[512] = {0};
    if(sprintf(greetingMsg, HELO_GREETING_MSG, parser->serverDom, parsedCmd) < 0){
        parser->machine->currentState = WELCOME;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = GREETING;
    parser->status = strdup(greetingMsg);
    parser->machine->loginState = EHLO;
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = EHLO;
    parser->structure->ehloDomain = strdup(parsedCmd);
    return SUCCESS;
}

static int greetingTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);
    toUpperCmd(command);

    if((strncmp(command, MAIL_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = MAIL_FROM_INPUT;
        return mailFromTransition(parser, mailArgs);
    }
    else if((strncmp(command, RCPT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        parser->machine->currentState = GREETING;
        parser->status = strdup(NEED_MAIL_FROM);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if((strncmp(command, DATA_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = GREETING;
        parser->status = strdup(NEED_RCPT_TO);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if((strncmp(command, RSET_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if((strncmp(command, VRFY_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if((strncmp(command, EXPN_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == ' ') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = EXPN;
        return ERR;
    }
    else if((strncmp(command, TRFM_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        if(parser->machine->loginState == HELO){
            parser->machine->currentState = GREETING;
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
    else if((strncmp(command, NOOP_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if((strncmp(command, QUIT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = QUIT_ST;
        parser->status = strdup(QUIT_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = QUIT;
        return TERMINAL;
    }
    else if(((strncmp(command, HELO_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE)
         || ((strncmp(command, EHLO_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE)) {
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

static int vrfyTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);

    char **result = NULL;
    int count;
    char parsedCmd[256] = {0};
    for(int i=0; i<256 && command[i] != '\0' && command[i] != '\r'; i++) parsedCmd[i] = command[i];

    int res = vrfy(parsedCmd, VALID_FILE, &result, &count);
    if(res == ERR) {
        parser->status = strdup(VRFY_NOT_FOUND);
        parser->machine->currentState = parser->machine->priorState;
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = VRFY;
        return ERR;
    }
    char buff[1024];
    if(count == 1) {
        for(int i = 0; result[0][i] != '\0'; i++) {
            if(result[0][i] == '\n') result[0][i] = '\0';
        }
        sprintf(buff, VRFY_OK_MSG, result[0]);
        freeValidMails(result, count);
        parser->status = strdup(buff);
        parser->machine->currentState = parser->machine->priorState;
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = VRFY;
        return SUCCESS;
    }

    int j = 0;
    for(int i = 0; i < count ; i++) {
        strcpy(buff + j, VRFY_AMBIGUOUS_INIT_LINE);
        j += strlen(VRFY_AMBIGUOUS_INIT_LINE);
        for(int k = 0; j<1024 && result[i][k] != '\n' ; j++, k++) {
            buff[j] = result[i][k];
        }
        buff[j++] = '>';
        buff[j++] = '\n';
    }

    buff[j-1] = '\0';
    buff[j] = '\0';
    char ret[1500] = {0};
    sprintf(ret, VRFY_AMBIGUOUS_MSG, buff);
    freeValidMails(result, count);
    parser->status = strdup(ret);
    parser->machine->currentState = parser->machine->priorState;
    parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
    parser->structure->cmd = VRFY;
    return SUCCESS;
}

static int mailFromTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);
    toUpperCmd(command);

    if(strncmp(command, FROM_ARG, FROM_ARG_LEN) != SUCCESS) {
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        parser->machine->currentState = GREETING;
        return ERR;
    }

    char * mailArg = command + FROM_ARG_LEN;
    unsigned long len = strlen(mailArg) - CLRF_LEN; // Size of the argument plus null (discards \r\n and the character '>')
    char parsedCmd[256] = {0};
    strncpy(parsedCmd, mailArg, len - 1);

    if(regexec(&mailRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS) == REG_NOMATCH){
        parser->machine->currentState = GREETING;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = MAIL_FROM_OK;
    parser->status = strdup(GENERIC_OK_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = MAIL_FROM;
    parser->structure->ehloDomain = strdup(parsedCmd);
    return SUCCESS;
}

static int mailFromOkTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);
    toUpperCmd(command);

    if((strncmp(command, RCPT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = RCPT_TO_INPUT;
        return rcptToTransition(parser, mailArgs);
    }
    else if((strncmp(command, MAIL_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(MAIL_FROM_ALREADY_IN);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if((strncmp(command, DATA_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(NEED_RCPT_TO);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if((strncmp(command, RSET_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if((strncmp(command, VRFY_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if((strncmp(command, EXPN_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
        parser->structure->cmd = EXPN;
        return ERR;
    }
    else if((strncmp(command, TRFM_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        if(parser->machine->loginState == HELO){
            parser->machine->currentState = MAIL_FROM_OK;
            parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
            parser->structure->cmd = TRFM;
            return ERR;
        }
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = TRFM;
        parser->transform = !parser->transform;
    }
    else if((strncmp(command, NOOP_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if((strncmp(command, QUIT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = QUIT_ST;
        parser->status = strdup(QUIT_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = QUIT;
        return TERMINAL;
    }
    else if((strncmp(command, HELO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')
         || (strncmp(command, EHLO_CMD, CMD_LEN) && command[CMD_LEN] == ' ')) {
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(ALREADY_SIGNED);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = MAIL_FROM_OK;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = ERROR;
    return ERR;
}

static int rcptToTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);

    toUpperCmd(command);

    if(strncmp(command, TO_ARG, TO_ARG_LEN) != SUCCESS) {
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        parser->machine->currentState = MAIL_FROM_OK;
        return ERR;
    }

    char * mailArg = command + TO_ARG_LEN;
    unsigned long len = strlen(mailArg) - CLRF_LEN; // Size of the argument plus null (discards \r\n and the character '>')
    char parsedCmd[512] = {0};
    strncpy(parsedCmd, mailArg, len - 1);

    if(regexec(&mailRegex, parsedCmd, NO_FLAGS, NULL, NO_FLAGS) == REG_NOMATCH){
        parser->machine->currentState = MAIL_FROM_OK;
        parser->status = strdup(PARAM_SYNTAX_ERROR_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = RCPT_TO_OK;
    parser->status = strdup(GENERIC_OK_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = RCPT_TO;
    parser->structure->ehloDomain = strdup(parsedCmd);
    return SUCCESS;
}


static int rcptToOkTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);
    toUpperCmd(command);

    if((strncmp(command, DATA_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r'){
        parser->machine->currentState = DATA_INPUT;
        parser->status = strdup(ENTER_DATA_MSG);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = DATA;
        return SUCCESS;
    }
    else if((strncmp(command, RCPT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        char * mailArgs = command + CMD_LEN + 1;
        parser->machine->currentState = RCPT_TO_INPUT;
        return rcptToTransition(parser, mailArgs);
    }
    else if((strncmp(command, MAIL_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE){
        parser->machine->currentState = RCPT_TO_OK;
        parser->status = strdup(MAIL_FROM_ALREADY_IN);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }
    else if((strncmp(command, RSET_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = GREETING;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = RSET;
        return SUCCESS;
    }
    else if((strncmp(command, VRFY_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = VRFY_PARAM;
        parser->machine->priorState = WELCOME;
        char * vrfyParam = command + CMD_LEN + 1;
        return vrfyTransition(parser, vrfyParam);
    }
    else if((strncmp(command, EXPN_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE) {
        parser->machine->currentState = RCPT_TO_OK;
        parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = EXPN;
        return ERR;
    }
    else if((strncmp(command, TRFM_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        if(parser->machine->loginState == HELO){
            parser->machine->currentState = RCPT_TO_OK;
            parser->status = strdup(CMD_NOT_IMPLEMENTED_MSG);
            parser->structure = malloc(sizeof(CommandStructure));
            parser->structure->cmd = TRFM;
            return ERR;
        }
        parser->machine->currentState = RCPT_TO_OK;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = TRFM;
        parser->transform = !parser->transform;
    }
    else if((strncmp(command, NOOP_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = RCPT_TO_OK;
        parser->status = strdup(GENERIC_OK_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = NOOP;
        return SUCCESS;
    }
    else if((strncmp(command, QUIT_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == '\r') {
        parser->machine->currentState = QUIT_ST;
        parser->status = strdup(QUIT_MSG);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = QUIT;
        return TERMINAL;
    }
    else if(((strncmp(command, HELO_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE)
         || ((strncmp(command, EHLO_CMD, CMD_LEN) == SUCCESS) && command[CMD_LEN] == SPACE)) {
        parser->machine->currentState = RCPT_TO_OK;
        parser->status = strdup(ALREADY_SIGNED);
        parser->structure = malloc(sizeof(CommandStructure));
        parser->structure->cmd = ERROR;
        return ERR;
    }

    parser->machine->currentState = RCPT_TO_OK;
    parser->status = strdup(SYNTAX_ERROR_MSG);
    parser->structure = malloc(sizeof(CommandStructure));
    parser->structure->cmd = ERROR;
    return ERR;
}

static int dataTransition(Parser parser, char * command) {
    if(parser->status != NULL) {
        free(parser->status);
        parser->status = NULL;
    }
    if(parser->structure != NULL) freeStruct(parser);

    if(strncmp(command, END_DATA, END_DATA_LEN) == SUCCESS) {
        parser->machine->currentState = GREETING;
        parser->status = strdup(QUEUED_MSG);
        parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
        parser->structure->cmd = DATA;
        parser->structure->dataStr = strdup(command);
        return SUCCESS;
    }

    parser->machine->currentState = DATA_INPUT;
    parser->status = NULL;
    parser->structure = (CommandStructure *) malloc(sizeof(CommandStructure));
    parser->structure->cmd = DATA;
    parser->structure->dataStr = strdup(command);
    return SUCCESS;
}

void toUpperCmd(char * command) {
    for(int i = 0; i < CMD_LEN && command[i] != '\0' && command[i] != '\n'; i++){
        command[i] = toupper(command[i]);
    }
}

static void freeStruct(Parser parser) {
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
    parser->structure = NULL;
}

/**
 * Initiates the Regex variables, needs to be called only once by the server
 * the first time it inits to set all the regex used in the server to parse
 * all the arguments given by the client.
 */
int compileRegexes(void) {
    if((regcomp(&ipv4Regex, IPV4_REGEX, REG_EXTENDED) != SUCCESS)     ||
       (regcomp(&ipv6Regex, IPV6_REGEX, REG_EXTENDED) != SUCCESS)     ||
       (regcomp(&domainRegex, DOMAIN_REGEX, REG_EXTENDED) != SUCCESS) ||
       (regcomp(&mailRegex, MAIL_REGEX, REG_EXTENDED) != SUCCESS)    ) return ERR;
    return SUCCESS;
}

/**
 * Allocates the necessary memory for the State Machine and
 * for the parser.
 */
Parser initParser(const char * serverDomain) {
    StateMachinePtr sm = malloc(sizeof(struct StateMachine));
    sm->currentState = WELCOME;
    Parser parser = malloc(sizeof(_Parser_t));
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
int parseCmd(Parser parser, char * command) {
    if(parser == NULL || parser->machine == NULL) return TERMINAL;
    switch(parser->machine->currentState){
        case WELCOME: return welcomeTransition(parser, command);
        case GREETING: return greetingTransition(parser, command);
        case MAIL_FROM_OK: return mailFromOkTransition(parser, command);
        case RCPT_TO_OK: return rcptToOkTransition(parser, command);
        case DATA_INPUT: return dataTransition(parser, command);
        default: return TERMINAL; // Unexpected parsing error, should never get here
    }
}

/**
 * Destroys the parser, this should be done if the server is
 * about to finish the service with the client, because all the
 * inner parser state will be lost.
 */
void destroy(Parser parser) {
    if(parser == NULL) return;
    free(parser->machine);
    if(parser->status != NULL) free(parser->status);
    if(parser->structure != NULL) freeStruct(parser);
    if(parser->serverDom != NULL) free(parser->serverDom);
    free(parser);
}

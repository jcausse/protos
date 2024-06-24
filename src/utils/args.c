/**
 * \file        args.c
 * \brief       Parse command-line arguments.
 * 
 * \author      Causse, Juan Ignacio
 * \author      Codagnone, Juan Francisco
 * \author      De Caro, Guido
 */

#include "args.h"

/*************************************************************************/
/* Constant, macro, and module-global variable definitions               */
/*************************************************************************/

#define PORT_MAX 65535

#define PRODUCT_NAME        "smtpd"
#define PRODUCT_VERSION     "0.1.0"

#define ORGANIZATION        "ITBA, Protocolos de Comunicacion"

#define COMPILATION_DATE    __DATE__
#define COMPILATION_TIME    __TIME__

#define TEAM_NO "3"

#define TEAM_MEMBERS(XX)                                                    \
    XX("Causse",        "Juan Ignacio",     "61105")                        \
    XX("De Caro",       "Guido",            "61590")                        \
    XX("Mindlin",       "Felipe",           "62774")                        \
    XX("Sendot",        "Francisco",        "62351")                        

static const char * team_members_last_names[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) LAST_NAME,
    TEAM_MEMBERS(XX)
    #undef XX
};

static const char * team_members_first_names[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) FIRST_NAME,
    TEAM_MEMBERS(XX)
    #undef XX
};

static const char * team_members_ids[] ={
    #define XX(LAST_NAME, FIRST_NAME, ID) ID,
    TEAM_MEMBERS(XX)
    #undef XX
};

// TODO ESTO SIRVE?????
/**
 * \typedef     Return values
 */
typedef enum {
    ARGS_OK                 = 0,    // No error
    ARGS_MISSING            = 1,    // Missing required parameter
    ARGS_TYPE_MISMATCH      = 2,    // Type mismatch for parameter that requires a type
    ARGS_TOO_MANY           = 3,    // Too many arguments for parameter
    ARGS_TOO_FEW            = 4,    // Too few arguments for parameter
} ArgsErrors;

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

bool parse_args(int argc, char ** argv, SMTPDArgs * const result){
    memset(result, 0, sizeof(SMTPDArgs));
    
    // \todo hardcoded
    result->mngr_port = 9090;
    result->smtp_port = 2525;
    
    return true;
}



/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

static void usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s -d <DOMAIN NAME > -m <MAIL DIRECTORY > -s <SMTP PORT > -p <MANAGEMENT PORT > [OPTION]...\n"
        "\n"
        "   -t   <COMMAND PATH >    What transformation command will be used.\n"
        "   -f   <VRFY DIR >        Directory where already verified mails are stored and new one will be stored.\n"
        "\n",
        progname);
    exit(1);
}

static void version(void) {
    fprintf(stdout, "%s v%s\n", PRODUCT_NAME, PRODUCT_VERSION);
    fprintf(stdout, "%s\n", ORGANIZATION);

    fprintf(stdout, "Authors:");
    unsigned int i = 0;
    while(team_members_last_names[i] != NULL){
        fprintf(stdout, "* %5s: %12s, %15s\n",
            team_members_ids[i],
            team_members_last_names[i],
            team_members_first_names[i]
        )
    }

    fprintf("Compiled on %s at %s\n", COMPILATION_DATE, COMPILATION_TIME);
}

static unsigned short port(const char *s) {
    char * end = NULL;
    const long sl = strtol(s, &end, 10);

    if (
            end == s 
        ||  * end != '\0'
        ||  ((LONG_MIN == sl || LONG_MAX == sl) && ERANGE == errno)
        ||  sl < 0 
        ||  sl > PORT_MAX
    ) {
        fprintf(stderr, "Invalid port: not in range [1-65535]: %s\n", s);
        exit(1);
    }

    return (unsigned short) sl;
}


/***********************************************************************************************/

/**
 * \note    The following code is part of the Library "libCinputs", that can be found at
 *          https://github.com/jcausse/libcinputs
 */

#define STRTOL_MIN_RADIX 2
#define STRTOL_MAX_RADIX 36

static short parse_short(const char* str, int radix){
    long res = parse_long(str, radix);
    if (res > SHRT_MAX || res < SHRT_MIN){
        errno = ERANGE;
        return (short) 0;
    }
    return (short) res;
}

static int parse_int(const char* str, int radix){
    long res = parse_long(str, radix);
    if (res > INT_MAX || res < INT_MIN){
        errno = ERANGE;
        return (int) 0;
    }
    return (int) res;
}

static long parse_long(const char* str, int radix){
    if (str == NULL || (radix != 0 && (radix < STRTOL_MIN_RADIX || radix > STRTOL_MAX_RADIX))){
        errno = EINVAL;
        return 0L;
    }

    errno = 0;
    char* end;
    bool range_error = false, non_valid_error = false;

    long parsed = strtol(str, &end, radix);

    if (end == str){
        non_valid_error = true;
        errno = EINVAL;
    }
    else{
        range_error = errno == ERANGE;
    }

    return range_error || non_valid_error ? 0L : parsed;
}

/***********************************************************************************************/

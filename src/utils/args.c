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
/* Private function declarations                                         */
/*************************************************************************/

/**
 * \brief                       Removes a substring from a string. Used to remove the TO_TRANSFORM substring from the user name.
 * 
 * \param[in] string            Given string to clean.
 * \param[in] sub               Substring to remove form the string.
 *
 * 
 * \return                      void
 */
static void removeSubstr (char *string, char *sub);

/**
 * \brief       Prit usage.
 * 
 * \param[in] progname      Program name (argv[0]).
 */
static void usage(const char *progname);

/**
 * \brief       Print product version, authors, compilation date and other messages.
 */
static void version(void);

/*************************************************************************/

/**
 * \note    The following code is part of the Library "libCinputs", that can be found at
 *          https://github.com/jcausse/libcinputs
 */

static short    parse_short     (const char* str, int radix);
static int      parse_int       (const char* str, int radix);
static long     parse_long      (const char* str, int radix);

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

parse_args(const int argc, char *argv, struct socks5argsargs) {
    memset(args, 0, sizeof(*args));

    args->socks_addr = "0.0.0.0";
    args->socks_port = 1080;

    args->mng_addr   = "127.0.0.1";
    args->mng_port   = 8080;

    args->disectors_enabled = true;

    int c;
    int nusers = 0;

    while (true) {
        int option_index = 0;
        static struct option long_options[] = {
            { 0,           0,                 0, 0 }
        };

        c = getopt_long(argc, argv, "hl:L:Np:P:u:v", long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'l':
                args->socks_addr = optarg;
                break;
            case 'L':
                args->mng_addr = optarg;
                break;
            case 'N':
                args->disectors_enabled = false;
                break;
            case 'p':
                args->socks_port = port(optarg);
                break;
            case 'P':
                args->mng_port   = port(optarg);
                break;
            case 'u':
                if(nusers >= MAX_USERS) {
                    fprintf(stderr, "maximun number of command line users reached: %d.\n", MAX_USERS);
                    exit(1);
                } else {
                    user(optarg, args->users + nusers);
                    nusers++;
                }
                break;
            case 'v':
                version();
                exit(0);
                break;
            default:
                fprintf(stderr, "unknown argument %d.\n", c);
                exit(1);
        }

    }
    if (optind < argc) {
        fprintf(stderr, "argument not accepted: ");
        while (optind < argc) {
            fprintf(stderr, "%s ", argv[optind++]);
        }
        fprintf(stderr, "\n");
        exit(1);
    }
}

/*************************************************************************/
/* Private function definitions                                          */
/*************************************************************************/

static void removeSubstr (char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
}

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
        );
    }

    fprintf("Compiled on %s at %s\n", COMPILATION_DATE, COMPILATION_TIME);
}

/*************************************************************************/

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

/*************************************************************************/

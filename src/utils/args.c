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

#define PRODUCT_VERSION     "0.1.0"

#define ORGANIZATION        "ITBA, Protocolos de Comunicacion"

#define COMPILATION_DATE    __DATE__
#define COMPILATION_TIME    __TIME__

#define TEAM_NO "3"

#define TEAM_MEMBERS(XX)                                    \
    XX("Causse",            "Juan Ignacio",     "61105")    \
    XX("De Caro",           "Guido",            "61590")    \
    XX("Mindlin",           "Felipe",           "62774")    \
    XX("Sendot",            "Francisco",        "62351")    \
    XX("Garcia Lauberer",   "Federico Inti",    "61374")

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

bool parse_args(int argc, char **argv, SMTPDArgs *const result) {
    int c;
    int flag = 0;
    if (argc < 7) {
        int option_index = 0;
        static struct option long_options[] = { { 0, 0, 0, 0 } };
        c = getopt_long(argc, argv, "hd:m:s:p:t:f:L:v", long_options, &option_index);
        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'v':
                version();
                exit(0);
                break;
            default:
                usage(argv[0]);
                exit(0);
        }
    }

    memset(result, 0, sizeof(SMTPDArgs));
    result->min_log_level = LOGGER_DEFAULT_MIN_LOG_LEVEL;
    while (true) {
        int option_index = 0;
        static struct option long_options[] = { { 0, 0, 0, 0 } };

        c = getopt_long(argc, argv, "hd:m:s:p:t:f:L:v", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'd':
                result->domain = optarg;
                flag ++;
                break;
            case 's':
                result->smtp_port = parse_short(optarg, 10);
                flag ++;
                break;
            case 'p':
                result->mngr_port = parse_short(optarg, 10);
                flag ++;
                break;
            case 't':
                result->trsf_cmd = optarg;
                result->trsf_enabled = true;
                break;
            case 'f':
                result->vrfy_mails = optarg;
                result->vryf_enabled = true;
                break;
            case 'L':
                if (optarg == NULL) {
                    fprintf(stderr, "missing argument for option -L\n");
                    return false;
                } 
                else if (strcmp(optarg, "1") == 0) {
                    result->min_log_level = LOGGER_LEVEL_INFO;
                } 
                else if (strcmp(optarg, "2") == 0) {
                    result->min_log_level = LOGGER_LEVEL_NORMAL;
                } 
                else if (strcmp(optarg, "3") == 0) {
                    result->min_log_level = LOGGER_LEVEL_CRITICAL;
                }
#ifdef __USE_DEBUG_LOGS__
                else if (strcmp(optarg, "0") == 0) {
                    result->min_log_level = LOGGER_LEVEL_DEBUG;
                }
#endif // __USE_DEBUG_LOGS__ 
                else {
                    fprintf(stderr, "invalid argument for option -L\n");
                    return false;
                }
                break;
            case 'v':
                version();
                exit(0);
                break;
            case 'l':
                result->log_file = optarg;
                flag ++;
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

    if(flag < 4){
        fprintf(stderr, "Error: Missing required arguments.\n");
        usage(argv[0]);
    }

    return true;
}




/*************************************************************************/
/* Private functions                                                     */
/*************************************************************************/

void usage(const char *progname) {
    fprintf(stderr,
        "Usage: %s -d <DOMAIN NAME > -s <SMTP PORT > -p <MANAGEMENT PORT > -l <LOG FILE DIRECTTORY > [OPTION]...\n"
        "\n"
        "   -h                      Print this help message and exit.\n"
        "   -t   <COMMAND PATH >    What transformation command will be used.\n"
        "   -f   <VRFY DIR >        Directory where already verified mails are stored and new one will be stored.\n"
        "   -L   <LOG_LEVEL >       Min log level.\n"
        "   -v                      Print version information and exit.\n"
        "\n",
        progname);
    exit(1);
}

void version(void) {
    fprintf(stdout, "%s v%s\n", PRODUCT_NAME, PRODUCT_VERSION);
    fprintf(stdout, "%s\n", ORGANIZATION);

    fprintf(stdout, "Authors:\n");
    unsigned int i = 0;
    while(team_members_last_names[i] != NULL){
        fprintf(stdout, "* %5s: %-20s %-15s\n",
            team_members_ids[i],
            team_members_last_names[i],
            team_members_first_names[i]
        );
        i++;
    }

    printf("Compiled on %s at %s\n", COMPILATION_DATE, COMPILATION_TIME);
}


/***********************************************************************************************/

/**
 * \note    The following code is part of the Library "libCinputs", that can be found at
 *          https://github.com/jcausse/libcinputs
 */

#define STRTOL_MIN_RADIX 2
#define STRTOL_MAX_RADIX 36

short parse_short(const char* str, int radix){
    long res = parse_long(str, radix);
    if (res > SHRT_MAX || res < SHRT_MIN){
        errno = ERANGE;
        return (short) 0;
    }
    return (short) res;
}

long parse_long(const char* str, int radix){
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


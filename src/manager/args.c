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

#define PRODUCT_VERSION     "0.0.0"

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


/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>






bool parse_args(int argc, char **argv, UDPArgs *const result) {
    int c;
    while ((c = getopt(argc, argv, "hi:p:v")) != -1) {
        switch (c) {
            case 'h':
                usage(argv[0]);
                break;
            case 'i':
                result->server_ip = optarg;
                break;
            case 'p':
                result->port = atoi(optarg);
                break;
            case 'v':
                fprintf(stdout, "Version info\n");
                version();
                exit(EXIT_SUCCESS);
                break;
            case '?':
                if (optopt == 'i' || optopt == 'p') {
                    fprintf(stderr, "Option -%c requires an argument.\n", optopt);
                } else  {
                    fprintf(stderr, "Unknown option `-%c'.\n", optopt);
                } 
                usage(argv[0]);
                return false;
                break;
            default:
                abort();
                return false;
        }
    }

    // Ensure mandatory options are set
    if (!result->server_ip || result->port == 0) {
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
        "Usage: %s -i <IP ADDRESS> -p <PORT>\n"
        "\n"
        "   -i <IP ADDRESS>   Server IP address\n"
        "   -p <PORT>         Server port number\n"
        "   -h                Print this help message and exit.\n"
        "   -v                Print version information and exit.\n"
        "\n",
        progname);
    exit(EXIT_FAILURE);
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


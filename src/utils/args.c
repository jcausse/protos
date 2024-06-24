/**
 * \file        args.c
 * \brief       Parse command-line arguments.
 */

#include "args.h"

#define PORT_MAX 65535

/*************************************************************************/
/* Public functions                                                      */
/*************************************************************************/

static char removeSubstr (char *string, char *sub) {
    char *match;
    int len = strlen(sub);
    while ((match = strstr(string, sub))) {
        *match = '\0';
        strcat(string, match+len);
    }
    return *string;
}

// TODO REFACTOR
bool parse_args(int argc, char ** argv, struct smtpd_args * const result){
/*
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
    */
    if(argc < 9){
        usage(argv[0]);
    }

    int arg = 1;

    char* cmd;
    char c; 
    while(arg < argc){
        cmd = argv[arg];
        c = removeSubstr(cmd, "-");
        switch (c)
        {
        case 'd':
            *result->domain = argv[arg + 1];
            arg += 2;
            break;
        case 'm':
            *result->mail_directory = argv[arg + 1];
            arg += 2;
            break;
        case 's':
            unsigned short port = port_check(argv[arg + 1]);
            *result->smtp_port = port;
            arg += 2;
            break;
        case 'p':
            unsigned short port = port_check(argv[arg + 1]);
            *result->mng_port = port;
            arg += 2;
            break;
        case 't':
            *result->trsf_cmd = argv[arg + 1];
            arg += 2;
            break;
        case 'f':
            *result->vrfy_mails = argv[arg + 1];
            arg += 2;
            break;
        default:
            break;
        }
    }
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


/**
 * \file        main.c
 * \brief       SMTPD server main file. Starts SMTPD and starts
 *              listening for client connections.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "lib/logger.h"
#include "utils/selector.h"
#include "utils/stats.h"
#include "utils/sockets.h"
#include "sock_types_handlers.h"
#include "lib/exceptions.h"
#include "messages.h"
#include "utils/args.h"
#include "utils/parser.h"
#include "utils/stats.h"
#include "utils/client_data.h"

#define BACKLOG_SIZE            10
#define CONFIG_LOG_FILE         "/home/juani/Desktop/smtpd.log" // \todo HARDCODED
#define MAX_BUFFER_SIZE 1049

/****************************************************************/
/* Global variables                                             */
/****************************************************************/

Logger      logger      = NULL;     // Logger (see src/lib/logger.h)
Selector    selector    = NULL;     // Selector (see src/utils/selector.h)
Stats       stats       = NULL;     // Stats (see src/utils/stats.h)

bool        transform_enabled = false;

/****************************************************************/
/* Extern global variables                                      */
/****************************************************************/

extern SockReadHandler  read_handlers[];
extern SockWriteHandler write_handlers[];

/****************************************************************/
/* Private function declarations                                */
/****************************************************************/

/**
 * \brief       Initializes SMTPD.
 *
 * \param[in] args      Command-line arguments
 */
static void smtpd_init(SMTPDArgs * args);

/**
 * \brief       Starts SMTPD after initialization. This function only returns if an error occurs.
 */
static void smtpd_start(void);

/**
 * \brief       Cleanup resources and exit with code `exit_code`.
 *
 * \param[in] exit_code Integer passed to exit (3).
 */
static void smtpd_cleanup(int exit_code);

/**
 * \brief       Abort execution. Performs cleanup and exites with code `EXIT_FAILURE`.
 */
static void smtpd_abort(void);

/**
 * \brief       Captures `SIGINT` signal to gracefully stop SMTPD.
 * \details     Attempts to perform a cleanup of the Selector and the Logger.
 *
 * \param[in] signum    According to the documentation, the handler receives the
 *                      signal number that triggered its call. In this case, `SIGINT`.
 */
void sigint_handler(int sigint);

/****************************************************************/
/* Main function                                                */
/****************************************************************/

// \todo revisar
#if 0
typedef struct{
    int pid;                    //Process pid
    int toSlavePipe[2];         //Pipes to send and receive messages to the slave
    int fromSlavePipe[2];
} SlaveInfo;

SlaveInfo create_transformer(char* command){

    SlaveInfo slave;

    if (pipe(slave.toSlavePipe) == -1 || pipe(slave.fromSlavePipe) == -1) {
        perror("pipe");
        slave.pid = -1;
        return slave;
    }

    if ((slave.pid = fork()) == 0) {
            // Código para el proceso hijo (slave)
        bool        transform_enabled;
        close(slave.toSlavePipe[1]);
        close(slave.fromSlavePipe[0]);

            // Redirecciona las entradas/salidas estándar según sea necesario
        dup2(slave.toSlavePipe[0], STDIN_FILENO);
        dup2(slave.fromSlavePipe[1], STDOUT_FILENO);
        char * args[] = {"./central.exe",command,NULL};
        execve("./central.exe",args, NULL);

        perror("execve");
        slave.pid = -1;
        return slave;

        } else if (slave.pid == -1) {
        perror("fork");
        slave.pid = -1;
        return slave;

        }else {
            // Código para el proceso padre
            close(slave.fromSlavePipe[1]);
            close(slave.toSlavePipe[0]);
        }
        return slave;
}

void free_transformer(SlaveInfo slave){
    close(slave.toSlavePipe[0]);
    close(slave.fromSlavePipe[1]);
    kill(slave.pid, SIGKILL);
}

int transform_mail(char* file_name, SlaveInfo central){
    char outputBuffer[MAX_BUFFER_SIZE];
    ssize_t nbytes;
    /*Send file to slave*/
    write(central.toSlavePipe[1], file_name, strlen(file_name) + 1);
    /*Get answer from the slave*/
    nbytes = read(central.fromSlavePipe[0], outputBuffer, sizeof(outputBuffer) - 1);
    if (nbytes > 0) {
        outputBuffer[nbytes] = '\0';  // Null-terminate the output buffer
        if (strcmp(outputBuffer, SUCCESS) == 0) {
            return 254;
        } else {
            return 255;
        }
    } else {
        perror("read from slave");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
#endif

int main(int argc, char ** argv){
    /* Parse command-line arguments */
    SMTPDArgs args;

// \todo revisar
#if 0
    SlaveInfo central;
#endif

    if (! parse_args(argc, argv, &args)){
        return EXIT_FAILURE;
    }

// \todo revisar
#if 0
    if(args.trsf_enabled == true){
        central = create_transformer(args.trsf_cmd);
    }
    transform_enabled = args.trsf_enabled;

    if(central.pid == -1){
        perror("Transformation central failure");
        return EXIT_FAILURE;
    }

    char * file_name;
    /* FIle names should be like userName-Nameformail.txt*/
    /*If there is a new mail to transform*/
    if(args.trsf_enabled == true){
        if(transform_mail(file_name, central) == 255){
            /*Informe user of failure*/
        }else{
            /*Informe user of success*/
        }
    }
    // \todo mover
#endif

    /* Initialize and start server */
    smtpd_init(&args);                  // Initialize SMTPD.
    smtpd_start();                      // Start SMTPD. Only returns on error.
    smtpd_abort();                      // Cleanup on error.
    return EXIT_FAILURE;                // Never reached.
}

/****************************************************************/
/* Private function definitions                                 */
/****************************************************************/

static void smtpd_init(SMTPDArgs * const args){
    /* Variables */
    int         sv_fd_4     = -1;       // IPv4 server socket
    int         sv_fd_6     = -1;       // IPv6 server socket
    int         mngr_fd     = -1;       // UDP management port

    /* Logger configuration */
    LoggerConfig logger_cfg = {
        .min_log_level      = args->min_log_level,  // Minimum log level
        .with_datetime      = true,                 // Include date and time in logs
        .with_level         = true,                 // Include log levels
        .flush_immediately  = true,                 // Disable buffering for real-time log viewing (tail -f)
        .log_prefix         = PRODUCT_NAME " v" PRODUCT_VERSION     // Product info as log prefix
    };

    /**
     * Directories used to store tmp and persistent mails
     * It is no necessary to check for error because it will
     * error iif the directories are already created.
     */
    mkdir(TMP, FILE_PERMISSIONS);
    mkdir(INBOX, FILE_PERMISSIONS);

    /* Status */
    bool comp_regex = false;

    TRY{
        /* Set SIGINT handler */
        signal(SIGINT, sigint_handler);

        /* Create Logger */
        THROW_IF(
            (logger =
                Logger_create(
                    logger_cfg,         // Logger configuration
                    CONFIG_LOG_FILE     // Absolute path to the file that will hold the logs
                )
            ) == NULL                   // Expected return: Logger (not NULL)
        );
        LOG_VERBOSE(MSG_INFO_LOGGER_CREATED);

        /* Close unused file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        /* Compile SMTP parser regexes */
        THROW_IF(compileRegexes() != SUCCESS);
        LOG_VERBOSE(MSG_INFO_REGEX_COMPILED);
        comp_regex = true;

        /* Create passive sockets (server sockets) for IPv4 and IPv6 */
        THROW_IF_NOT(
            tcp_serve(
                args->smtp_port,        // Port for SMTPD
                BACKLOG_SIZE,           // Max quantity of pending (unaccepted) connections
                &sv_fd_4,               // IPv4 socket (output parameter)
                &sv_fd_6                // IPv6 socket (output parameter)
            )
        );                              // Expected return: true
        LOG_VERBOSE(MSG_INFO_SV_SOCKET_CREATED, args->smtp_port);

        /* Create management socket for both IPv4 and IPv6 */
        THROW_IF_NOT(
            udp_serve(
                args->mngr_port,        // Management port
                &mngr_fd                // Management socket (output parameter)
            )
        );                              // Expected return: true
        LOG_VERBOSE(MSG_INFO_MNG_SOCKET_CREATED, args->mngr_port);

        /* Create Stats */
        THROW_IF((stats = Stats_init()) == NULL);
        LOG_VERBOSE(MSG_INFO_STATS_CREATED);

        /* Create Selector */
        THROW_IF((selector = Selector_create(free)) == NULL) // \todo data free fn
        LOG_VERBOSE(MSG_INFO_SELECTOR_CREATED);

        /* Add both of the server sockets and the manager socket to the Selector */
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_4,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER4,      // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sv_fd_4, SOCK_TYPE_SERVER4);
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                sv_fd_6,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_SERVER6,      // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK              // Expected return: SELECTOR_OK
        );
        LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sv_fd_6, SOCK_TYPE_SERVER6);
        THROW_IF_NOT(
            Selector_add(
                selector,               // The Selector itself
                mngr_fd,                // File descriptor to add
                SELECTOR_READ,          // Mode
                SOCK_TYPE_MANAGER,      // File descriptor type
                NULL                    // No data needed
            )
            == SELECTOR_OK
        );
        LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, mngr_fd, SOCK_TYPE_MANAGER);
    }
    CATCH{
        /* Could not create the logger */
        if (logger == NULL){
            if (errno == EACCES){
                fprintf(stderr, MSG_ERR_EACCES, CONFIG_LOG_FILE);
            }
            else if (errno == ENOMEM){
                fprintf(stderr, MSG_ERR_NO_MEM);
            }
            fprintf(stderr, MSG_EXIT_FAILURE);
        }

        /* Could not compile regex */
        else if (! comp_regex){
            LOG_ERR(MSG_ERR_REGEX_COMPILATION);
        }

        /* Could not create server socket */
        else if (sv_fd_4 == -1 || sv_fd_6 == -1){
            LOG_ERR(MSG_ERR_SV_SOCKET);
        }

        /* Could not create management socket */
        else if (mngr_fd == -1){
            LOG_ERR(MSG_ERR_MNGR_SOCKET);
        }

        /* Could not initialize Stats */
        else if (stats == NULL){
            LOG_ERR(MSG_ERR_STATS_CREATION);
        }

        /* Could not create Selector */
        else if (selector == NULL){
            LOG_ERR(MSG_ERR_SELECTOR_CREATION);
        }

        /* No memory available for allocation */
        else{
            LOG_ERR(MSG_ERR_NO_MEM);
        }

        /* Cleanup and exit */
        safe_close(sv_fd_4);
        safe_close(sv_fd_6);
        safe_close(mngr_fd);
        smtpd_abort();
    }
}

static void smtpd_start(void){
    while (true){
        /* Perform a select (2) operation */
        LOG_DEBUG(MSG_DEBUG_SELECTOR_SELECT);
        SelectorErrors err = Selector_select(selector);     // Blocking
        if (err != SELECTOR_OK){

            /* Abort on Select error */
            if (err == SELECTOR_SELECT_ERR){
                LOG_ERR(MSG_ERR_SELECT);
            }
            return;
        }

        /* Iterate through all ready file descriptors */
        int     sock_fd;
        int     sock_type;
        void *  sock_data;
        while ((sock_fd = Selector_read_next(selector, &sock_type, &sock_data)) != SELECTOR_NO_FD){

            /* Prevent errors from invalid socket types */
            if (sock_type < 0 || sock_type >= SOCK_TYPE_QTY){
                Selector_remove(selector, sock_fd, SELECTOR_READ_WRITE, true);
                LOG_ERR(MSG_ERR_UNK_SOCKET_TYPE, sock_fd, sock_type);
                continue;
            }

            /* Call handler for that socket type */
            LOG_DEBUG(MSG_DEBUG_SOCKET_READY, sock_fd, sock_type, "READ");
            HandlerErrors ret = read_handlers[sock_type](sock_fd, sock_data);

            /* Abort on no memory */
            if (ret == HANDLER_NO_MEM){
                LOG_ERR(MSG_ERR_NO_MEM);
                return;
            }
        }
        while ((sock_fd = Selector_write_next(selector, &sock_type, &sock_data)) != SELECTOR_NO_FD){

            /* Prevent errors from invalid socket types */
            if (sock_type < 0 || sock_type >= SOCK_TYPE_QTY){
                Selector_remove(selector, sock_fd, SELECTOR_READ_WRITE, true);
                LOG_ERR(MSG_ERR_UNK_SOCKET_TYPE, sock_fd, sock_type);
                continue;
            }

            /* Call handler for that socket type */
            LOG_DEBUG(MSG_DEBUG_SOCKET_READY, sock_fd, sock_type, "WRITE");
            HandlerErrors ret = write_handlers[sock_type](sock_fd, sock_data);

            /* Abort on no memory */
            if (ret == HANDLER_NO_MEM){
                LOG_ERR(MSG_ERR_NO_MEM);
                return;
            }
        }
    }
}

static void smtpd_cleanup(int exit_code){
    Selector_cleanup(selector);     // NULL-safe
    Logger_cleanup(logger);         // NULL-safe
    Stats_cleanup(stats);           // NUll-safe
    exit(exit_code);
}

static void smtpd_abort(void){
    LOG_ERR(MSG_EXIT_FAILURE);
    smtpd_cleanup(EXIT_FAILURE);
}

void sigint_handler(int signum){
    (void) signum;                  // Avoids unused parameter warning
    LOG_MSG(MSG_EXIT_SIGINT);       // NULL-safe
    smtpd_cleanup(EXIT_SUCCESS);    // No error occurred
}

/*
... Inicio
compileRegexes();
...
Parser * parser = initParser(serverDomain);
client->parser = parser;
... fetch command
int retStatus = parseCmd(parser, cmd);
if(retStatus == TERMINAL){
  ... free resources
}
if(retStatus == ERR){
  writeClient(parser->status);
}
// the ret value is SUCCESS
CommandStructure *str = parser->structure;
switch(str->cmd){
  case(HELO): persistDomain(str->heloDomain); break;
  case(EHLO): persistEhloDomain(str->ehloDomain); break;
  case(MAIL_FROM): writeInDomain(str->mailFromStr); break;
  ...
}
writeClient(parser->status);
 */

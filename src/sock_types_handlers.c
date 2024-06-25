/**
 * \file        sock_types_handlers.c
 * \brief       Handlers for all socket types used by the server.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 * \author      Mindlin, Felipe
 * \author      Sendot, Francisco
 */

#include "sock_types_handlers.h"
#include "utils/manager_parser.h"
#include "utils/client_data.h"
#include "domain.h"
#include "utils/sockets.h"

#define CLOSED 0
#define MANAGER_READ_BUFF_SIZE 15
#define MAX_DIR_SIZE 512 // Out file system has a 2-level directory to save the mails
#define REL_TMP "../tmp"
#define REL_INBOX "../inbox"
#define RW_FOPEN "r+"

#define SERVER_ERROR "421-%s Service not available, closing transmission channel.\r\n"
#define MAIL_FROM_STR "MAIL FROM: <%s>\r\n"
#define RCPT_TO_STR "RCPT TO: <%s>\r\n"
#define DATA_STR "DATA\r\n"
#define DOT_CLRF ".\r\n"
#define LITERAL_STR "%s"

/***********************************************************************************************/
/* Global variables                                                                            */
/***********************************************************************************************/

static MngrCommand              current_manager_cmd;
static struct sockaddr_storage  manager_addr;
static socklen_t                manager_addr_len;

/***********************************************************************************************/
/* Extern global variables                                                                     */
/***********************************************************************************************/

extern Logger       logger;
extern Selector     selector;
extern Stats        stats;

extern bool         transform_enabled;
extern char *       domain;

/***********************************************************************************************/
/* Read / Write handler pointer arrays                                                         */
/***********************************************************************************************/

/**
 * Read handlers for each socket type
 */
SockReadHandler read_handlers[] = {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_read_handler,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    NULL
};

/**
 * Write handlers for each socket type
 */
SockWriteHandler write_handlers[] = {
    #define XX(sock_type_numeric, sock_read_handler, sock_write_handler) sock_write_handler,
    SOCK_TYPES_AND_HANDLERS(XX)
    #undef XX
    NULL
};

/***********************************************************************************************/
/* Private helper declarations                                                                 */
/***********************************************************************************************/
char *strdup(const char *s);

// Clears the first offset bytes in the array and reallocates
// the string at the beggining of the array
static int clearBuff(int offset, char * buff);

#define RESPONSE_SIZE 15

// static const char * get_cmd_string(MngrCommand cmd);

/***********************************************************************************************/
/* Read handler definitions                                                                    */
/***********************************************************************************************/

HandlerErrors handle_server4 (int fd, void * _){
    (void) _;

    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    bool done = false;

    do {
        errno = 0;
        int sock = -1;

        /* Attempt to accept a connection */
        do {
            sock = accept(fd, (struct sockaddr *) &addr, &addr_len);
        } while (errno == EINTR);

        /* If there was a connection to be accepted */
        if (sock != -1){
            /* Create client data */
            ClientData data = malloc(sizeof(_ClientData_t));
            for(int i = 0; i < READ_BUFF_SIZE; i++) {
                data->r_buff[i] = '\0';
                data->w_buff[i] = '\0';
            }
            data->parser = initParser(domain);
            data->receiverMails = (char **) malloc(sizeof(char *));
            data->receiverMailsAmount = 0;
            data->r_count = 0;
            data->w_count = 0;
            data->clientDomain = NULL;
            data->senderMail = NULL;
            data->fileName = NULL;
            data->mailFile = NULL;

            /* Add the accepted connection's fd to the Selector */
            SelectorErrors ret = Selector_add(
                selector,
                sock,
                SELECTOR_WRITE,
                SOCK_TYPE_CLIENT,
                data
            );
            if (ret == SELECTOR_NO_MEMORY){
                close(sock);
                FREE_PTR(free, data->receiverMails);
                FREE_PTR(free, data);
                return HANDLER_NO_MEM;
            }
            LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sock, SOCK_TYPE_CLIENT);

            /* Create log */
            char ip [INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(addr.sin_addr), ip, INET_ADDRSTRLEN);
            uint16_t port = ntohs(addr.sin_port);
            LOG_MSG(MSG_NEW_CLIENT, ip, port);

            /* Increment statistics */
            Stats_increment(stats, STATKEY_CONNS);
            Stats_increment(stats, STATKEY_CURR_CONNS);
        }

        /* No more connections pending */
        else if (errno ==  EAGAIN || errno == EWOULDBLOCK){
            done = true;
        }
    } while (!done);

    return HANDLER_OK;
}

HandlerErrors handle_server6 (int fd, void * _){
    (void) _;

    struct sockaddr_in6 addr;
    socklen_t addr_len = sizeof(addr);
    bool done = false;

    do {
        errno = 0;
        int sock = -1;

        /* Attempt to accept a connection */
        do {
            sock = accept(fd, (struct sockaddr *) &addr, &addr_len);
        } while (errno == EINTR);

        /* If there was a connection to be accepted */
        if (sock != -1){
            /* Create client data */
            ClientData data = malloc(sizeof(_ClientData_t));
            for(int i = 0; i < READ_BUFF_SIZE; i++) {
                data->r_buff[i] = '\0';
                data->w_buff[i] = '\0';
            }
            data->parser = initParser(domain);
            data->receiverMails = (char **) malloc(sizeof(char *));
            data->receiverMailsAmount = 0;
            data->r_count = 0;
            data->w_count = 0;
            data->clientDomain = NULL;
            data->senderMail = NULL;
            data->fileName = NULL;
            data->mailFile = NULL;

            /* Add the accepted connection's fd to the Selector */
            SelectorErrors ret = Selector_add(
                selector,
                sock,
                SELECTOR_WRITE,
                SOCK_TYPE_CLIENT,
                data
            );
            if (ret == SELECTOR_NO_MEMORY){
                close(sock);
                FREE_PTR(free, data->receiverMails);
                FREE_PTR(free, data);
                return HANDLER_NO_MEM;
            }
            LOG_DEBUG(MSG_DEBUG_SELECTOR_ADD, sock, SOCK_TYPE_CLIENT);

            /* Create log */
            char ip [INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, &(addr.sin6_addr), ip, INET6_ADDRSTRLEN);
            uint16_t port = ntohs(addr.sin6_port);
            LOG_MSG(MSG_NEW_CLIENT, ip, port);

            /* Increment statistics */
            Stats_increment(stats, STATKEY_CONNS);
            Stats_increment(stats, STATKEY_CURR_CONNS);
        }

        /* No more connections pending */
        else if (errno ==  EAGAIN || errno == EWOULDBLOCK){
            done = true;
        }
    } while (!done);

    return HANDLER_OK;
}

/**
 * \todo
 */
HandlerErrors handle_client_read (int fd, void * data){
    ClientData clientData = (ClientData) data;
    char buff[READ_BUFF_SIZE] = {0};

    ssize_t bytes = recv(fd, buff, READ_BUFF_SIZE, MSG_DONTWAIT);
    if(bytes == CLOSED) {
        Selector_remove(selector, fd, SELECTOR_READ_WRITE, true);
        safe_close(fd);
        return HANDLER_OK;
    }
    else if(bytes == ERR && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return HANDLER_OK;
    }

    char aux[READ_BUFF_SIZE] = {0};

    if(clientData->r_count != 0) {
        size_t i = 0;
        while(i < clientData->r_count){
            aux[i] = clientData->r_buff[i];
            i++;
        }
        clientData->r_count = clearBuff(i, clientData->r_buff);
    }

    size_t read = strlen(aux);
    int i = 0;

    while(i < bytes && buff[i] != '\n' && buff[i] != '\0') {
        aux[i + read] = buff[i];
        i++;
    }

    if(i != bytes || aux[i + read] == '\0') { // Not a full command
        for(int j = 0; j < bytes ; j++) {
            clientData->r_buff[clientData->r_count] = aux[j];
            clientData->r_count++;
        }
        return HANDLER_OK;
    }
    int ret = parseCmd(clientData->parser, aux);
    if(ret == TERMINAL) {
        // State on the client has been achieved, need to free
        // file resources, discard temp files created that are not
        // useful and return a closing message to the client
        // DO NOT CLOSE THE SELECTOR_WRITE UNTIL THE CLOSING CONNECTION
        // MESSAGE IS SENT
        strcpy(clientData->w_buff, clientData->parser->status);
        Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
        Selector_remove(selector, fd, SELECTOR_READ, false);
        return HANDLER_OK;
    }
    if(ret == ERR) {
        // Not new info has to be processed, all it is needed is
        // to inform the user the error it has in handle_client_write
        strcpy(clientData->w_buff, clientData->parser->status);
        clientData->w_count = strlen(clientData->w_buff);
        Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
        Selector_remove(selector, fd, SELECTOR_READ, false);
        return HANDLER_OK;
    }

    // TODO - Handle custom parser return info (create files
    // and persist any important info such as directories
    // and mail info sent by the client
    CommandStructure * structure = clientData->parser->structure;
    switch(structure->cmd) {
        case HELO: clientData->clientDomain = strdup(structure->heloDomain); break;
        case EHLO: clientData->clientDomain = strdup(structure->ehloDomain); break;
        case MAIL_FROM: clientData->senderMail = strdup(structure->mailFromStr); break;
        case RCPT_TO: {
            clientData->receiverMails[clientData->receiverMailsAmount] = strdup(structure->rcptToStr);
            clientData->receiverMailsAmount++;
            char fileName[MAX_DIR_SIZE] = {0};

            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            sprintf(fileName, "%s/From:%s-%d-%02d-%02d %02d:%02d:%02d", REL_TMP, clientData->senderMail, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
            clientData->mailFile = fopen(fileName, RW_FOPEN);
            if(clientData->mailFile == NULL) {
                // Server error, should close the connection
                char buff[WRITE_BUFF_SIZE] = {0};
                sprintf(buff, SERVER_ERROR, clientData->clientDomain);
                strcpy(clientData->w_buff, buff);
                Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
                Selector_remove(selector, fd, SELECTOR_READ, false);
                safe_close(fd);
                return HANDLER_OK;
            }
            fprintf(clientData->mailFile, MAIL_FROM_STR, clientData->senderMail);
            fprintf(clientData->mailFile, RCPT_TO_STR, clientData->receiverMails[clientData->receiverMailsAmount-1]);
            fprintf(clientData->mailFile, DATA_STR); // The next state will be the data
            break;
        }
        case DATA: {
            if(strcmp(structure->dataStr, DOT_CLRF) == SUCCESS) {
                // Handle transform
            }
            else{
                fprintf(clientData->mailFile, LITERAL_STR,structure->dataStr);
            }
        }
        default: break;
    }

    // Status to inform the client to the client
    strcpy(clientData->w_buff, clientData->parser->status);
    clientData->w_count = strlen(clientData->w_buff);


    Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_READ, false);

    return HANDLER_OK;
}

HandlerErrors handle_manager_read (int fd, void * data){
    (void) data;

    /* Local variables */
    uint8_t buffer[MANAGER_READ_BUFF_SIZE];
    struct sockaddr_in manager_addr;
    socklen_t manager_addr_len = sizeof(manager_addr);
    ssize_t read_bytes;

    /* Attempt to read from socket */
    read_bytes = recvfrom(fd, buffer, MANAGER_READ_BUFF_SIZE, MSG_DONTWAIT,
                          (struct sockaddr *) &manager_addr, &manager_addr_len);
    
    LOG_VERBOSE("Received %ld bytes from manager", read_bytes);

    /* If an error occurred, return */
    if (read_bytes == -1){
        return HANDLER_NO_OP;
    }

    /* Parse read message */
    MngrCommand cmd;
    if (! manager_parse(buffer, (size_t) read_bytes, &cmd)){
        LOG_VERBOSE(" Manager sent an invalid command.");
        return HANDLER_NO_OP;
    }


    Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_READ, false);

    return HANDLER_OK;
}

/***********************************************************************************************/
/* Write handler definitions                                                                   */
/***********************************************************************************************/

/**
 * \todo
 */
HandlerErrors handle_client_write (int fd, void * data){
    ClientData clientData = (ClientData) data;

    if(clientData->w_count < 1){
        //return HANDLER_OK;
    }


    if(clientData->parser->status == NULL){
        return HANDLER_OK;
    }

    ssize_t bytes = send(fd, clientData->parser->status, strlen(clientData->parser->status), MSG_DONTWAIT);
    if(bytes == CLOSED) {
        Selector_remove(selector, fd, SELECTOR_READ_WRITE, true);
        safe_close(fd);
        return HANDLER_OK;
    }
    else if(bytes == ERR && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return HANDLER_OK;
    }

    Selector_add(selector, fd, SELECTOR_READ, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_WRITE, false);

    // Clear buffer
    //clientData->w_count = clearBuff(bytes, clientData->w_buff);
    return HANDLER_OK;
}

HandlerErrors handle_manager_write(int fd, void *data) {
    (void) data;

    uint8_t response[RESPONSE_SIZE] = {0};
    StatVal statval;

    /* Set protocol signature and version in the response */
    response[0] = 0xFF;
    response[1] = 0xFE;
    response[2] = 0x00;

    /* Set identifier */
    uint16_t identifier = 0x1234;
    response[3] = (identifier >> 8) & 0xFF;
    response[4] = identifier & 0xFF;

    switch (current_manager_cmd) {
        case CMD_CONEX_HISTORICAS:
            response[5] = 0x00;  // Status: Success
            response[14] = 0x00; // Boolean: 0 (FALSE)

            Stats_get(stats, STATKEY_CONNS, &statval);
            memcpy(&(response[6]), &(statval), sizeof(uint64_t));

            break;

        case CMD_CONEX_CONCURRENTES:
            response[5] = 0x00;  // Status: Success
            response[14] = 0x00; // Boolean: 0 (FALSE)

            Stats_get(stats, STATKEY_CURR_CONNS, &statval);
            memcpy(&(response[6]), &statval, sizeof(uint64_t));

            break;

        case CMD_BYTES_TRANSFERIDOS:
            response[5] = 0x00;  // Status: Success
            response[14] = 0x00; // Boolean: 0 (FALSE)

            Stats_get(stats, STATKEY_TRANSF_BYTES, &statval);
            memcpy(&(response[6]), &statval, sizeof(uint64_t));

            break;

        case CMD_ESTADO_TRANSFORMACIONES:
            response[5] = 0x00;  // Status: Success
            response[14] = transform_enabled ? 0x01 : 0x00; // Transformation status as boolean
            break;

        case CMD_TRANSFORMACIONES_ON:
            response[5] = 0x00;  // Status: Success
            response[14] = 0x01; // Boolean: 1 (TRUE)

            transform_enabled = true;

            break;

        case CMD_TRANSFORMACIONES_OFF:
            response[5] = 0x00;  // Status: Success
            response[14] = 0x00; // Boolean: 0 (FALSE)

            transform_enabled = false;

            break;

        default:
            response[5] = 0x03;  // Status: Invalid command
            response[14] = 0x00; // Boolean: 0 (FALSE)
            break;
    }

    sendto(
        fd,
        response,
        RESPONSE_SIZE * sizeof(response[0]),
        MSG_DONTWAIT,
        (struct sockaddr *) &manager_addr,
        manager_addr_len
    );

    Selector_add(selector, fd, SELECTOR_READ, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_WRITE, false);

    return HANDLER_OK;
}

/***********************************************************************************************/
/* Private helper definitions                                                                  */
/***********************************************************************************************/

static int clearBuff(int offset, char * buff) {
    LOG_DEBUG("beforeClear: %s", buff);
    int i = 0;
    while(offset < WRITE_BUFF_SIZE && buff[offset] != '\0') {
        buff[i] = buff[offset];
        buff[offset] = '\0';
        i++;
        offset++;
    }
    LOG_DEBUG("afterClear: %s", buff);
    while(i < offset) buff[i] = '\0';
    return strlen(buff);
}

// static const char * get_cmd_string(MngrCommand cmd){
//     switch(cmd){
//         case CMD_CONEX_HISTORICAS:
//             return "connections_historic";
//         case CMD_CONEX_CONCURRENTES:
//             return "connections_current";
//         case CMD_BYTES_TRANSFERIDOS:
//             return "transferred_bytes";
//         case CMD_ESTADO_TRANSFORMACIONES:
//             return "transform_state_get";
//         case CMD_TRANSFORMACIONES_ON:
//             return "transform_state_set_true";
//         case CMD_TRANSFORMACIONES_OFF:
//             return "transform_state_set_false";
//         default:
//             break;
//     }
//     return "";
// }

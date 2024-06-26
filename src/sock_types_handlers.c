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
#include "utils/transform.h"

#define CLOSED 0
#define MANAGER_READ_BUFF_SIZE 15
#define REL_TMP "../tmp"
#define REL_INBOX "../inbox"
#define RW_FOPEN "a+"

#define SERVER_ERROR "421-%s Server error.\r\n"
#define LITERAL_STR "%s"
#define DEFAULT_TMP_MAIL "%s/From:%s %d-%02d-%02d %02d:%02d:%02d"
#define DEFAULT_MAIL_NAME "From:%s %d-%02d-%02d %02d:%02d:%02d"

#define DOT_CLRF ".\r\n"

#define MAX_DIR_SIZE 512 // Out file system has a 2-level directory to save the mails
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
extern char        *transform_cmd;

extern bool        vrfy_enabled;
extern char        *vrfy_mails;

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
//static int clearBuff(int offset, char * buff);

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
            for(int i = 0; i < BUFF_SIZE; i++) {
                data->r_buff[i] = '\0';
            }
            buffer_init(&data->buffer, BUFF_SIZE, data->r_buff);
            data->parser = initParser(domain);
            data->receiverMails = (char **) malloc(sizeof(char *));
            data->receiverMailsAmount = 0;
            data->clientDomain = NULL;
            data->senderMail = NULL;
            data->mailFile = NULL;
            data->mailPath = NULL;
            data->closedMailFd = SUCCESS;
            data->parser->vrfyAllowed = vrfy_enabled;
            data->parser->transformAllowed = transform_enabled;

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
            for(int i = 0; i < BUFF_SIZE; i++) {
                data->r_buff[i] = '\0';
            }
            buffer_init(&data->buffer, BUFF_SIZE, data->r_buff);
            data->parser = initParser(domain);
            data->receiverMails = (char **) malloc(sizeof(char *));
            data->receiverMailsAmount = 0;
            data->clientDomain = NULL;
            data->senderMail = NULL;
            data->mailFile = NULL;
            data->mailPath = NULL;
            data->closedMailFd = SUCCESS;
            data->parser->vrfyAllowed = vrfy_enabled;
            data->parser->transformAllowed = transform_enabled;

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
HandlerErrors handle_client_read(int fd, void * data){
    ClientData clientData = (ClientData) data;
    char buff[BUFF_SIZE] = {0};

    ssize_t bytes = recv(fd, buff, BUFF_SIZE, MSG_DONTWAIT);
    if(bytes == CLOSED) {
        LOG_VERBOSE("Connection ended");
        Stats_decrement(stats, STATKEY_CURR_CONNS);
        Selector_remove(selector, fd, SELECTOR_READ_WRITE, true);
        safe_close(fd);
        return HANDLER_OK;
    }
    else if(bytes == ERR && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return HANDLER_OK;
    }

    Stats_update(stats, STATKEY_TRANSF_BYTES, bytes); // Increment transferred bytes by the number of bytes read

    bool readyToParse = false;

    for(int i = 0; i < bytes && buff[i] != '\0'; i++){
        if(buff[i] == '\n'){
            readyToParse = true;
        }
        buffer_write(&clientData->buffer, buff[i]);
    }

    if(!readyToParse) {
        return HANDLER_OK;
    }

    for(int i = 0; i < BUFF_SIZE ;i++) {
        buff[i] = '\0';
    }

    int i = 0;
    char c;
    while((c = buffer_read(&clientData->buffer)) != '\n'){
        buff[i++] = c;
    }
    buff[i] = c;

    buffer_compact(&clientData->buffer);

    int ret = parseCmd(clientData->parser, buff);
    if(ret == TERMINAL) {
        clientData->parser->structure->cmd = QUIT;
        Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
        Selector_remove(selector, fd, SELECTOR_READ, false);
        return HANDLER_OK;
    }
    if(ret == ERR) {
        Selector_add(selector, fd, SELECTOR_WRITE, - 1, NULL);
        Selector_remove(selector, fd, SELECTOR_READ, false);
        return HANDLER_OK;
    }

    CommandStructure * structure = clientData->parser->structure;
    switch(structure->cmd) {
        case HELO: clientData->clientDomain = strdup(structure->heloDomain); break;
        case EHLO: clientData->clientDomain = strdup(structure->ehloDomain); break;
        case MAIL_FROM: {
            clientData->senderMail = strdup(structure->mailFromStr);
            if(clientData->mailFile == NULL){
                char fileName[MAX_DIR_SIZE] = {0};

                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(fileName, DEFAULT_TMP_MAIL, TMP, clientData->senderMail, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                if(clientData->mailPath != NULL){
                    free(clientData->mailPath);
                }
                clientData->mailPath = strdup(fileName);
            }
            break;
        }
        case RCPT_TO: {
            clientData->receiverMails[clientData->receiverMailsAmount] = strdup(structure->rcptToStr);
            clientData->receiverMailsAmount++;
            clientData->receiverMails = realloc(clientData->receiverMails, sizeof(char*)*(clientData->receiverMailsAmount + 1));
            break;
        }
        case DATA: {
            if(structure->dataStr != NULL && strncmp(structure->dataStr, DOT_CLRF, strlen(DOT_CLRF)) == SUCCESS) {

                time_t t = time(NULL);
                struct tm tm = *localtime(&t);

                char filename[MAX_DIR_SIZE] = {0};
                sprintf(filename, DEFAULT_MAIL_NAME, clientData->senderMail, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
                clientData->closedMailFd = fclose(clientData->mailFile);

                if(clientData->parser->transform && transform_enabled) {
                    int ret = transform(transform_cmd, clientData->mailPath);
                    if(ret == ERR) {
                        sprintf(buff, SERVER_ERROR, clientData->clientDomain);
                        if(clientData->parser->status != NULL) free(clientData->parser->status);
                        clientData->parser->status = strdup(buff);
                        for(int i = 0; i < clientData->receiverMailsAmount ;i++) free(clientData->receiverMails[i]);
                        clientData->receiverMailsAmount = 0;
                        remove(clientData->mailPath);
                        free(clientData->mailPath);
                        Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
                        Selector_remove(selector, fd, SELECTOR_READ, false);
                        return HANDLER_OK;

                    }
                }

                for(int i = 0; i < clientData->receiverMailsAmount ;i++){
                    int ret = dump(clientData->mailPath, clientData->receiverMails[i], clientData->senderMail, filename);
                    if(ret == ERR) {
                        sprintf(buff, SERVER_ERROR, clientData->clientDomain);
                        if(clientData->parser->status != NULL) free(clientData->parser->status);
                        for(int i = 0; i < clientData->receiverMailsAmount ;i++) free(clientData->receiverMails[i]);
                        clientData->receiverMailsAmount = 0;
                        remove(clientData->mailPath);
                        free(clientData->mailPath);
                        Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
                        Selector_remove(selector, fd, SELECTOR_READ, false);
                        return HANDLER_OK;
                    }
                }
                for(int i = 0; i < clientData->receiverMailsAmount ;i++) free(clientData->receiverMails[i]);
                clientData->receiverMailsAmount = 0;
                remove(clientData->mailPath);
                free(clientData->mailPath);
            }
            else if(structure->dataStr != NULL){
                fprintf(clientData->mailFile, LITERAL_STR, structure->dataStr);
            }
            else {
                clientData->mailFile = fopen(clientData->mailPath, RW_FOPEN);
                if(clientData->mailFile == NULL) {
                    char buff[BUFF_SIZE] = {0};
                    sprintf(buff, SERVER_ERROR, clientData->clientDomain);
                    clientData->parser->status = strdup(buff);
                    rollBack(clientData->parser);
                    free(clientData->senderMail);
                    Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
                    Selector_remove(selector, fd, SELECTOR_READ, false);
                    return HANDLER_OK;
                }
                clientData->closedMailFd = 1;
            }
        }
        default: break;
    }

    Selector_add(selector, fd, SELECTOR_WRITE, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_READ, false);
    return HANDLER_OK;
}



HandlerErrors handle_manager_read(int fd, void * data) {
    (void) data;

    /* Local variables */
    uint8_t buffer[MANAGER_READ_BUFF_SIZE];
    // struct sockaddr_in manager_addr;
    // socklen_t manager_addr_len = sizeof(manager_addr);
    ssize_t read_bytes;

    /* Attempt to read from socket */
    manager_addr_len = sizeof(manager_addr); // Ensure manager_addr_len is properly initialized
    read_bytes = recvfrom(
        fd,
        buffer,
        MANAGER_READ_BUFF_SIZE * sizeof(buffer[0]),
        MSG_DONTWAIT,
        (struct sockaddr *) &manager_addr,
        &manager_addr_len
    );

    LOG_VERBOSE("Received %ld bytes from manager", read_bytes);

    /* If an error occurred, return */
    if (read_bytes == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            LOG_VERBOSE("No data available for reading (EAGAIN or EWOULDBLOCK)");
            return HANDLER_NO_OP;
        } else {
            LOG_VERBOSE("recvfrom failed");
            return HANDLER_NO_OP;
        }
    }

    /* Parse read message */
    MngrCommand cmd;
    if (!manager_parse(buffer, (size_t) read_bytes, &cmd)) {
        LOG_VERBOSE("Manager sent an invalid command.");
        return HANDLER_NO_OP;
    }
    LOG_VERBOSE("DETECTED %d\n", current_manager_cmd = cmd);

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
HandlerErrors handle_client_write(int fd, void * data){
    ClientData clientData = (ClientData) data;

    if(clientData->parser->status == NULL){
        Selector_add(selector, fd, SELECTOR_READ, -1, NULL);
        Selector_remove(selector, fd, SELECTOR_WRITE, false);
        return HANDLER_OK;
    }

    ssize_t bytes = send(fd, clientData->parser->status, strlen(clientData->parser->status), MSG_DONTWAIT);
    if(bytes == CLOSED) {
        LOG_VERBOSE("Connection ended");
        Stats_decrement(stats, STATKEY_CURR_CONNS);
        Selector_remove(selector, fd, SELECTOR_READ_WRITE, true);
        safe_close(fd);
        return HANDLER_OK;
    }
    else if(bytes == ERR && (errno == EAGAIN || errno == EWOULDBLOCK)) {
        return HANDLER_OK;
    }

    Stats_update(stats, STATKEY_TRANSF_BYTES, bytes); // Increment transferred bytes by the number of bytes sent

    if(clientData->parser->structure != NULL &&
        clientData->parser->structure->cmd == QUIT) {
        Selector_remove(selector, fd, SELECTOR_WRITE, true);
        safe_close(fd);
        return HANDLER_OK;
    }

    Selector_add(selector, fd, SELECTOR_READ, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_WRITE, false);
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
    LOG_VERBOSE("command: %d", current_manager_cmd);
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
        RESPONSE_SIZE,
        MSG_DONTWAIT,
        (struct sockaddr *) &manager_addr,
        manager_addr_len
    );
    LOG_VERBOSE("%d", manager_addr_len);
    LOG_VERBOSE("cmd = %d", current_manager_cmd);

    Selector_add(selector, fd, SELECTOR_READ, -1, NULL);
    Selector_remove(selector, fd, SELECTOR_WRITE, false);

    return HANDLER_OK;
}

/***********************************************************************************************/
/* Private helper definitions                                                                  */
/***********************************************************************************************/

/*static int clearBuff(int offset, char * buff) {
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
// }*/

#include <stdio.h>         // Standard input-output header
#include <stdlib.h>        // Standard library header (includes exit functions)
#include <string.h>        // String manipulation functions
#include <unistd.h>        // UNIX standard function definitions
#include <arpa/inet.h>     // Definitions for internet operations
#include <inttypes.h>      // Integer types and formatting macros
#include <sys/socket.h>    // Main sockets library

#include "manager.h"       // Protocol definitions
#include "args.h"          // Argument parsing functions
#define BUF_SIZE 1024      // Buffer size for input


// Structure for the request
struct Request {
    uint8_t signature[2];   // Protocol signature
    uint8_t version;        // Protocol version
    uint16_t identifier;    // Request identifier
    uint8_t auth[8];        // Authentication data
    MngrCommand command;    // Command
};

// Structure for the response
struct Response {
    uint8_t signature[2];   // Protocol signature
    uint8_t version;        // Protocol version
    uint16_t identifier;    // Response identifier
    uint8_t status;         // Status of the response
    uint64_t cantidad;      // Data quantity
    uint8_t booleano;       // Boolean flag
};

// Function prototypes
static void send_request(int sockfd, const struct sockaddr *addr, socklen_t addrlen, struct Request *req);
static void receive_response(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct Response *res);
static void print_menu();

int main(int argc, char *argv[]) {
    UDPArgs args;
    if(argc < 2){
        fprintf(stderr, "Usage: %s -i <server_ip> -p <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    // Parse command-line arguments
    if (!parse_args(argc, argv, &args)) {
        fprintf(stderr, "Failed to parse arguments.\n");
        exit(EXIT_FAILURE);
    }

    char *server_ip = args.server_ip;
    int port = args.port;

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    int sockfd;
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    char input[BUF_SIZE];
    int command;
    struct Response res;
    socklen_t addrlen = sizeof(server_addr);

    struct Request req = {
        { PROTOCOL_SIGNATURE_1, PROTOCOL_SIGNATURE_2 },
        0x00,
        htons(0x1234),
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },
        CMD_CONEX_HISTORICAS
    };

    while (1) {
        print_menu();
        fgets(input, sizeof(input), stdin);
        sscanf(input, "%d", &command);

        if (command < 0 || command > 7) {
            printf("Invalid command. Please select a number from 0 to 7.\n");
            continue;
        }

        req.command = (MngrCommand)command;

        send_request(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr), &req);

        receive_response(sockfd, (struct sockaddr *)&server_addr, &addrlen, &res);

        printf("\nReceived response:\n");
        printf("Status: %u\n", res.status);
        printf("Amount: %" PRIu64 "\n", res.cantidad);
        printf("Boolean: %u\n\n", res.booleano);

        if (command == CMD_TRANSFORMACIONES_OFF || command == CMD_TRANSFORMACIONES_ON) {
            printf("Transformation status = %s\n", res.booleano ? "ON" : "OFF");
        }
    }

    close(sockfd);
    return 0;
}

// Function to send request to the server
static void send_request(int sockfd, const struct sockaddr *addr, socklen_t addrlen, struct Request *req) {
    uint8_t buffer[14];  // Buffer for the request
    buffer[0] = req->signature[0];
    buffer[1] = req->signature[1];
    buffer[2] = req->version;
    buffer[3] = (req->identifier >> 8) & 0xFF;
    buffer[4] = req->identifier & 0xFF;
    memcpy(buffer + 5, req->auth, 8);
    buffer[13] = req->command;

    // Send the request
    if (sendto(sockfd, buffer, sizeof(buffer), 0, addr, addrlen) != sizeof(buffer)) {
        perror("sendto failed");
        exit(EXIT_FAILURE);
    }
}

// Function to receive response from the server
static void receive_response(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct Response *res) {
    uint8_t buffer[15];  // Buffer for the response
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, addr, addrlen);

    // Check if the received length is correct
    if (n != sizeof(buffer)) {
        fprintf(stderr, "Error: Response received with incorrect length.\n");
        exit(EXIT_FAILURE);
    }

    res->signature[0] = buffer[0];
    res->signature[1] = buffer[1];
    res->version = buffer[2];
    res->identifier = (buffer[3] << 8) | buffer[4];
    res->status = buffer[5];
    res->cantidad = ((uint64_t)buffer[6] << 56) |
                    ((uint64_t)buffer[7] << 48) |
                    ((uint64_t)buffer[8] << 40) |
                    ((uint64_t)buffer[9] << 32) |
                    ((uint64_t)buffer[10] << 24) |
                    ((uint64_t)buffer[11] << 16) |
                    ((uint64_t)buffer[12] << 8) |
                    (uint64_t)buffer[13];
    res->booleano = buffer[14];
}

// Function to print the command menu
static void print_menu() {
    printf("\nCommand Menu:\n");
    printf("0. Number of historical connections\n");
    printf("1. Number of concurrent connections\n");
    printf("2. Number of bytes transferred\n");
    printf("3. Check transformation status\n");
    printf("4. Transformations ON\n");
    printf("5. Transformations OFF\n");
    printf("Select a command (0-5): ");
}

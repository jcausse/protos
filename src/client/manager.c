#include <stdio.h>         // Standard input-output header
#include <stdlib.h>        // Standard library header (includes exit functions)
#include <string.h>        // String manipulation functions
#include <unistd.h>        // UNIX standard function definitions
#include <arpa/inet.h>     // Definitions for internet operations
#include <inttypes.h>      // Integer types and formatting macros
#include <sys/socket.h>    // Main sockets library

#define BUF_SIZE 1024      // Buffer size for input

// Protocol definitions
#define PROTOCOL_SIGNATURE_1 0xFF  // First byte of protocol signature
#define PROTOCOL_SIGNATURE_2 0xFE  // Second byte of protocol signature

/*

+----+------------+--------------+------------+-----------------+-----------------+
|SIG1| SIG2       | VERSION      | IDENTIFIER | AUTHENTICATION  | COMMAND         |
|    |            |              |            | (8 bytes)       |                 |
+----+------------+--------------+------------+-----------------+-----------------+
| 1  | 1          | 1            | 2          | 8               | 1               |
+----+------------+--------------+------------+-----------------+-----------------+


*/

// Possible commands
enum Command {
    CMD_CONEX_HISTORICAS = 0x00,         // Historical connections count command
    CMD_CONEX_CONCURRENTES = 0x01,       // Concurrent connections count command
    CMD_BYTES_TRANSFERIDOS = 0x02,       // Bytes transferred count command
    CMD_ESTADO_TRANSFORMACIONES = 0x03,  // Check transformations status command
    CMD_TRANSFORMACIONES_ON = 0x04,      // Enable transformations command
    CMD_TRANSFORMACIONES_OFF = 0x05,     // Disable transformations command
    CMD_VERIFY_ON = 0x06,                // Enable verify command
    CMD_VERIFY_OFF = 0x07                // Disable verify command
};

// Possible responses
enum Status {
    STATUS_SUCCESS = 0x00,                  // Success status
    STATUS_AUTH_FAILED = 0x01,              // Authentication failure status
    STATUS_INVALID_VERSION = 0x02,          // Invalid version status
    STATUS_INVALID_COMMAND = 0x03,          // Invalid command status
    STATUS_INVALID_REQUEST_LENGTH = 0x04,   // Invalid request length status
    STATUS_UNEXPECTED_ERROR = 0x05          // Unexpected error status
};

// Structure for the request
struct Request {
    uint8_t signature[2];   // Protocol signature
    uint8_t version;        // Protocol version
    uint16_t identifier;    // Request identifier
    uint8_t auth[8];        // Authentication data
    enum Command command;   // Command
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
void send_request(int sockfd, const struct sockaddr *addr, socklen_t addrlen, struct Request *req);
void receive_response(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct Response *res);
void print_menu();

int main(int argc, char *argv[]) {
    if (argc != 3) {
        // Print usage message and exit if incorrect number of arguments
        fprintf(stderr, "Usage: %s <server_ip> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *server_ip = argv[1];  // Server IP address
    int port = atoi(argv[2]);   // Server port number

    struct sockaddr_in server_addr;  // Server address structure
    memset(&server_addr, 0, sizeof(server_addr));  // Zero out the structure
    server_addr.sin_family = AF_INET;  // Set address family to Internet
    server_addr.sin_port = htons(port);  // Set port number

    // Convert IP address from text to binary form
    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton failed");
        exit(EXIT_FAILURE);
    }

    int sockfd;  // Socket file descriptor
    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Variables for menu handling
    char input[BUF_SIZE];  // Input buffer
    int command;  // Command variable
    struct Response res;  // Response structure
    socklen_t addrlen = sizeof(server_addr);  // Address length

    // Structure for the request
    struct Request req = {
        { PROTOCOL_SIGNATURE_1, PROTOCOL_SIGNATURE_2 },  // Protocol signature
        0x00,  // Version
        htons(0x1234),  // Request identifier (hardcoded)
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },  // Authentication data (hardcoded)
        CMD_CONEX_HISTORICAS  // Initial command for example
    };

    while (1) {
        print_menu();  // Show menu
        fgets(input, sizeof(input), stdin);  // Get user input
        sscanf(input, "%d", &command);  // Parse command

        // Validate user selection
        if (command < 0 || command > 7) {
            printf("Invalid command. Please select a number from 0 to 7.\n");
            continue;
        }

        req.command = (enum Command)command;  // Update command in the request

        // Send request to the server
        send_request(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr), &req);

        // Receive and process the server response
        receive_response(sockfd, (struct sockaddr *)&server_addr, &addrlen, &res);

        // Display received response
        printf("\nReceived response:\n");
        printf("Status: %u\n", res.status);
        printf("Amount: %" PRIu64 "\n", res.cantidad);
        printf("Boolean: %u\n\n", res.booleano);

        // Display additional information based on command
        if (command == CMD_TRANSFORMACIONES_OFF || command == CMD_TRANSFORMACIONES_ON) {
            printf("Transformation status = %s\n", res.booleano ? "ON" : "OFF");
        } else if (command == CMD_VERIFY_OFF || command == CMD_VERIFY_ON) {
            printf("Verify Status = %s\n", res.booleano ? "ON" : "OFF");
        }
    }

    close(sockfd);  // Close the socket
    return 0;
}

// Function to send request to the server
void send_request(int sockfd, const struct sockaddr *addr, socklen_t addrlen, struct Request *req) {
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
void receive_response(int sockfd, struct sockaddr *addr, socklen_t *addrlen, struct Response *res) {
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
void print_menu() {
    printf("\nCommand Menu:\n");
    printf("0. Number of historical connections\n");
    printf("1. Number of concurrent connections\n");
    printf("2. Number of bytes transferred\n");
    printf("3. Check transformation status\n");
    printf("4. Transformations ON\n");
    printf("5. Transformations OFF\n");
    printf("6. Verify ON\n");
    printf("7. Verify OFF\n");
    printf("Select a command (0-7): ");
}

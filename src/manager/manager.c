#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <sys/socket.h>

#include "manager.h"
#include "args.h"

#define LITTLE_ENDIAN 1
#define BIG_ENDIAN 2
#define UNKNOWN_ENDIAN 0
#define BUF_SIZE 1024
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

int check_endian(){
    unsigned int x = 0x12345678;
    unsigned char *c = (unsigned char*)&x;
    int endian;
    if (*c == 0x78) {
        endian = LITTLE_ENDIAN;
    } else if (*c == 0x12) {
        endian = BIG_ENDIAN;
    } else {
        endian = UNKNOWN_ENDIAN;
    }

    return endian;
}

uint64_t switch_endian(uint64_t x){
    int endian = check_endian();
    if(endian == LITTLE_ENDIAN) {
        return ((x & 0x00000000000000FFULL) << 56) |
               ((x & 0x000000000000FF00ULL) << 40) |
               ((x & 0x0000000000FF0000ULL) << 24) |
               ((x & 0x00000000FF000000ULL) << 8)  |
               ((x & 0x000000FF00000000ULL) >> 8)  |
               ((x & 0x0000FF0000000000ULL) >> 24) |
               ((x & 0x00FF000000000000ULL) >> 40) |
               ((x & 0xFF00000000000000ULL) >> 56);
    }else {
        return x;
    }
}

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

    struct Request req = {
        { PROTOCOL_SIGNATURE_1, PROTOCOL_SIGNATURE_2 },
        0x00,
        htons(0x1234),
        { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 },
        CMD_CONEX_HISTORICAS
    };

    struct Response res;
    socklen_t addrlen = sizeof(server_addr);

    while (1) {
        print_menu();
        
        char input[BUF_SIZE];
        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error reading input.\n");
            continue;
        }
        int command;
        if (sscanf(input, "%d", &command) != 1) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (command < 0 || command > 5) {
            printf("Invalid command. Please select a number from 0 to 1.\n");
            continue;
        }

        req.command = (MngrCommand)command;

        send_request(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr), &req);

        receive_response(sockfd, (struct sockaddr *)&server_addr, &addrlen, &res);

        printf("\nReceived response:\n");
        printf("Status: %u\n", res.status);
        printf("Amount: %" PRIu64 "\n", res.cantidad);
        printf("Boolean: %u\n\n", res.booleano);

        if (command == CMD_TRANSFORMACIONES_OFF || command == CMD_TRANSFORMACIONES_ON || command == CMD_ESTADO_TRANSFORMACIONES) {
            printf("Transformation status = %s\n", res.booleano ? "ON" : "OFF");
        }
    }

    close(sockfd);
    return 0;
}

// Function to send request to the server
static void send_request(int sockfd, const struct sockaddr *addr, socklen_t addrlen, struct Request *req) {
    uint8_t buffer[15];
    buffer[0] = req->signature[0];
    buffer[1] = req->signature[1];
    buffer[2] = req->version;

    if(check_endian() == LITTLE_ENDIAN){
        uint16_t id = htons(req->identifier);
        buffer[3] = (id >> 8) & 0xFF;
        buffer[4] = id & 0xFF;
    }else{
        buffer[3] = (req->identifier >> 8) & 0xFF;
        buffer[4] = req->identifier & 0xFF;
    }
    
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
    uint8_t buffer[BUF_SIZE];
    printf(" * ");
    int n = recvfrom(sockfd, buffer, sizeof(buffer), 0, addr, addrlen);

    printf("Received %d bytes\n", n);
    // Check if the received length is correct
    if (n < 15) {
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
    res->cantidad = switch_endian(res->cantidad);
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

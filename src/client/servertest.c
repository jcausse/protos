#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <inttypes.h>  // Para el formato PRIu64

#define PORT 9090
#define BUF_SIZE 1024

// Example metrics
int historic_connections = 1000;
int current_connections = 5;
int bytes_transferred = 123456;
struct timeval start_time;
int verification = 0;
char transformation_program[256] = "";

void handle_request(char *buffer, char *response) {
    struct timeval current_time;
    gettimeofday(&current_time, NULL);
    long uptime = current_time.tv_sec - start_time.tv_sec;

    // Configurar la firma del protocolo y la versión en la respuesta
    response[0] = 0xFF;
    response[1] = 0xFE;
    response[2] = 0x00;

    uint16_t identifier = (buffer[3] << 8) | buffer[4]; // Obtener el identificador del request
    response[3] = (identifier >> 8) & 0xFF;
    response[4] = identifier & 0xFF;

    switch (buffer[13]) { // Comando está en el byte 13 del buffer
        case 0x00: // cantidad de conexiones históricas
            {
                uint64_t cantidad = htobe64(historic_connections); // Convertir a Big Endian
                response[5] = 0x00; // Success
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x00; // Booleano: 0 (FALSE)
                break;
            }
        case 0x01: // cantidad de conexiones concurrentes
            {
                uint64_t cantidad = htobe64(current_connections); // Convertir a Big Endian
                response[5] = 0x00; // Success
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x00; // Booleano: 0 (FALSE)
                break;
            }
        case 0x02: // cantidad de bytes transferidos
            {
                uint64_t cantidad = htobe64(bytes_transferred); // Convertir a Big Endian
                response[5] = 0x00; // Success
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x00; // Booleano: 0 (FALSE)
                break;
            }
        case 0x03: // Ver estado de transformaciones
            {
                uint64_t cantidad = 0; // No se utiliza
                response[5] = 0x00; // Success
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = transformation_program[0] ? 0x01 : 0x00; // Estado binario
                break;
            }
        case 0x04: // Transformaciones ON
            {
                strcpy(transformation_program, "Transformaciones ON");
                response[5] = 0x00; // Success
                uint64_t cantidad = 0; // No se utiliza
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x01; // Booleano: 1 (TRUE)
                break;
            }
        case 0x05: // Transformaciones OFF
            {
                strcpy(transformation_program, "");
                response[5] = 0x00; // Success
                uint64_t cantidad = 0; // No se utiliza
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x00; // Booleano: 0 (FALSE)
                break;
            }
        case 0x06: // Verify ON
            {
                verification = 1;
                response[5] = 0x00; // Success
                uint64_t cantidad = 0; // No se utiliza
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x01; // Booleano: 1 (TRUE)
                break;
            }
        case 0x07: // Verify OFF
            {
                verification = 0;
                response[5] = 0x00; // Success
                uint64_t cantidad = 0; // No se utiliza
                memcpy(response + 6, &cantidad, sizeof(uint64_t));
                response[14] = 0x00; // Booleano: 0 (FALSE)
                break;
            }
        default:
            response[5] = 0x03; // Invalid command
            uint64_t cantidad = 0; // No se utiliza
            memcpy(response + 6, &cantidad, sizeof(uint64_t));
            response[14] = 0x00; // Booleano: 0 (FALSE)
    }
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUF_SIZE];
    char response[15];
    socklen_t addr_len = sizeof(client_addr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    memset(&client_addr, 0, sizeof(client_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    gettimeofday(&start_time, NULL);

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int n = recvfrom(sockfd, buffer, BUF_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) {
            perror("recvfrom failed");
            continue;
        }

        // Construir respuesta según el protocolo
        memset(response, 0, sizeof(response));
        handle_request(buffer, response);

        // Enviar respuesta al cliente
        sendto(sockfd, response, sizeof(response), 0, (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}

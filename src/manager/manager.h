#ifndef __MANAGER_H__
#define __MANAGER_H__

// Protocol definitions
#define PROTOCOL_SIGNATURE_1 0xFF  // First byte of protocol signature
#define PROTOCOL_SIGNATURE_2 0xFE  // Second byte of protocol signature

/*

Request
                +----+------------+--------------+------------+-----------------+-----------------+
Field           |SIG1| SIG2       | VERSION      | IDENTIFIER | AUTHENTICATION  | COMMAND         |
                +----+------------+--------------+------------+-----------------+-----------------+
Size (bytes)    | 1  | 1          | 1            | 2          | 8               | 1               |
                +----+------------+--------------+------------+-----------------+-----------------+

Response
                +----+------------+----------+--------------+-----------+------------+-------------
Field           |SIG1| SIG2       | VERSION  | IDENTIFIER   | STATUS    | QUANTITY   | BOOLEAN    |
                +----+------------+----------+--------------+-----------+------------+-------------
Size (bytes)    | 1  | 1          | 1        | 2            | 1         | 8          | 1          |
                +----+------------+----------+--------------+-----------+------------+-------------


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

#endif // __MANAGER_H__


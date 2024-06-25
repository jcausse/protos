/**
 * \file        selector.h
 * \brief       Selector allows monitoring of multiple file descriptors at
 *              the same time, useful for non-blocking socket applications.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 * \author      Mindlin, Felipe
 */

#include "manager_parser.h"

bool manager_parse(const uint8_t *buff, size_t len, MngrCommand *cmd) {
    // Check for minimum length required for a valid message
    if (len < 15) { // Minimum message length is 15 bytes
        return false;
    }

    // Validate protocol signature, version, identifier, and authentication
    if (buff[0] != PROTOCOL_SIGNATURE_1 || buff[1] != PROTOCOL_SIGNATURE_2) {
        return false; // Invalid protocol signature
    }

    if (buff[2] != PROTOCOL_VERSION) {
        return false; // Invalid protocol version
    }

    //uint16_t identifier = (buff[3] << 8) | buff[4];
    // Check identifier if needed for your protocol

    uint8_t expected_auth[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    if (memcmp(buff + 5, expected_auth, 8) != 0) {
        return false; // Invalid authentication data
    }

    // Extract the command byte
    uint8_t command_byte = buff[14]; // Command is located at byte 14

    // Validate and assign the command
    switch (command_byte) {
        case CMD_CONEX_HISTORICAS:
        case CMD_CONEX_CONCURRENTES:
        case CMD_BYTES_TRANSFERIDOS:
        case CMD_ESTADO_TRANSFORMACIONES:
        case CMD_TRANSFORMACIONES_ON:
        case CMD_TRANSFORMACIONES_OFF:
        case CMD_VERIFY_ON:
        case CMD_VERIFY_OFF:
            *cmd = (MngrCommand)command_byte;
            return true;
        default:
            return false; // Unsupported command
    }
}
/**
 * \file        selector.h
 * \brief       Selector allows monitoring of multiple file descriptors at
 *              the same time, useful for non-blocking socket applications.
 *
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 * \author      Mindlin, Felipe
 */

#ifndef __MANAGER_PARSER_H__
#define __MANAGER_PARSER_H__

#include <stdbool.h>                // bool, true, false
#include <stdint.h>                 // uint8_t, uint16_t
#include <stddef.h>                 // size_t
#include "../manager/manager.h"     // Manager protocol definitions


/***********************************************************************/

/**
 * \brief       Parse a command from the manager.
 * 
 * \param[in]  buff     Buffer to read the message from.
 * \param[in]  len      Length of the data present in the buffer.
 * \param[out] cmd      Pointer to store the parsed command.
 * 
 * \return      true on success, false otherwise.
 */
bool manager_parse (const uint8_t * __restrict__ buff, size_t len, MngrCommand * const cmd);

#endif // __MANAGER_PARSER_H__

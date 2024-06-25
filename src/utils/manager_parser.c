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

bool manager_parse (const uint8_t * __restrict__ buff, size_t len, MngrCommand * const cmd){
    (void) buff;
    (void) cmd;
    // \todo
    return true;
}
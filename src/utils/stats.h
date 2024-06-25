/**
 * \file        stats.h
 * \brief       SMTPD statistics.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#ifndef __STATS_H__
#define __STATS_H__

#include <stdlib.h>     // calloc(), free()
#include <stdbool.h>    // bool, true, false

/*************************************************************************/

/**
 * \typedef     Stats: Main Statistics ADT data type.
 */
typedef struct _Stats_t * Stats;

/**
 * \enum        StatKey: Keys of all possible statistics. STATKEY_START and STATKEY_END
 *                       are not valid statistic keys.
 */
typedef enum{
    STATKEY_CONNS,          // All connections since the server started
    STATKEY_CURR_CONNS,     // Current connection count
    STATKEY_TRANSF_BYTES,   // Total transferred bytes
} StatKey;

/**
 * \typedef     StatVal: Value type of a statistic.
 */
typedef long StatVal;

/*************************************************************************/

/**
 * \brief       Initialize statistics
 * 
 * \return      A new Stats reference on success, NULL on failure.
 */
Stats Stats_init();

/**
 * \brief       Get the value of a statistic.
 * 
 * \param[in]  self     The Stats object itself.
 * \param[in]  key      Statistic to get (as in `StatKey` enumeration).
 * \param[out] val      Value of the statistic. Pointed data is left unchanged on error.
 * 
 * \return      Returns `true` on success, or `false` if `key` does not represent a valid
 *              statistic key (as in `StatKey` enumeration).
 */
bool Stats_get(Stats const self, StatKey key, StatVal * const val);

/**
 * \brief       Update a statistic by adding the value of `delta` to it.
 * 
 * \param[in]  self     The Stats object itself.
 * \param[in]  key      Statistic to update (as in `StatKey` enumeration).
 * \param[in]  delta    Delta to be added to the specified statistic.
 * 
 * \return      Returns `true` on success, or `false` if `key` does not represent a valid
 *              statistic key (as in `StatKey` enumeration).
 */
bool Stats_update(Stats const self, StatKey key, StatVal delta);

/**
 * \brief       Add 1 to the target statistic. This function is equivalent to
 *              `Stats_update(self, key, 1)`.
 * 
 * \param[in]  self     The Stats object itself.
 * \param[in]  key      Statistic to increment (as in `StatKey` enumeration).
 * 
 * \return      Returns `true` on success, or `false` if `key` does not represent a valid
 *              statistic key (as in `StatKey` enumeration).
 */
bool Stats_increment(Stats const self, StatKey key);

/**
 * \brief       Substract 1 from the target statistic. This function is equivalent to
 *              `Stats_update(self, key, -1)`.
 * 
 * \param[in]  self     The Stats object itself.
 * \param[in]  key      Statistic to decrement (as in `StatKey` enumeration).
 * 
 * \return      Returns `true` on success, or `false` if `key` does not represent a valid
 *              statistic key (as in `StatKey` enumeration).
 */
bool Stats_decrement(Stats const self, StatKey key);

/**
 * \brief       Cleanup the `Stats` object, and free all allocated memory.
 * 
 * \param[in]  self     The Stats object itself.
 */
void Stats_cleanup(Stats const self);

/*************************************************************************/

#endif // __STATS_H__

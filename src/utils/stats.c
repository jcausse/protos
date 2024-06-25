/**
 * \file        stats.c
 * \brief       SMTPD statistics.
 * 
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
 */

#include "stats.h"

typedef struct _Stats_t{
    StatVal conns;
    StatVal curr_conns;
    StatVal transf_bytes;
} _Stats_t;

/**
 * \brief       Get the pointer of the statistic corresponding to the
 *              provided `key`.
 * 
 * \param[in] self      The Stats object itself.
 * \param[in] key       The statistic to get.
 * 
 * \return      A pointer to the statistic on success (`StatVal *`), `NULL`
 *              on failure.
 */
static StatVal * get_stat_ptr(Stats self, StatKey key) {
    if (self == NULL){
        return NULL;
    }
    switch (key) {
        case STATKEY_CONNS: 
            return &(self->conns);
        case STATKEY_CURR_CONNS: 
            return &(self->curr_conns);
        case STATKEY_TRANSF_BYTES: 
            return &(self->transf_bytes);
        default: 
            return NULL;
    }
}

Stats Stats_init(){
    Stats self = calloc(1, sizeof(struct _Stats_t));
    return self;
}

bool Stats_get(Stats const self, StatKey key, StatVal * const val){
    if (self == NULL || val == NULL){
        return false;
    }
    StatVal * ptr = get_stat_ptr(self, key);
    if (ptr == NULL){
        return false;
    }
    * val = * ptr;
    return true;
}

bool Stats_update(Stats const self, StatKey key, StatVal delta){
    if (self == NULL){
        return false;
    }
    StatVal * ptr = get_stat_ptr(self, key);
    if (ptr == NULL){
        return false;
    }
    *ptr += delta;
    return true;
}

bool Stats_increment(Stats const self, StatKey key) {
    return Stats_update(self, key, 1);
}

bool Stats_decrement(Stats const self, StatKey key) {
    return Stats_update(self, key, -1);
}

void Stats_cleanup(Stats const self){
    if (self == NULL){
        return;
    }
    free(self);
}

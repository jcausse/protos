/**
 * \file        exceptions.h
 * \details     Macros for OOP exceptions-like error handling in C.
 * \date        June, 2024
 * \author      Causse, Juan Ignacio (jcausse@itba.edu.ar)
*/


#ifndef __EXCEPTIONS_H__
#define __EXCEPTIONS_H__

/* Exception handling macros */
#define TRY                             \
    int __exception_thrown__ = 0;       \
    {                                   \
        do

#define CATCH                           \
        while(0);                       \
    }                                   \
    __exception_catch__:                \
    if (__exception_thrown__)

#define THROW_IF(expr)                  \
        if((expr)){                     \
            __exception_thrown__ = 1;   \
            goto __exception_catch__;   \
        }

#define THROW_IF_NOT(expr)              \
        if (! (expr)){                  \
            __exception_thrown__ = 1;   \
            goto __exception_catch__;   \
        }

#define FREE_PTR(free_fn, ptr)          \
        if ((ptr) != NULL){             \
            free_fn((ptr));             \
        }

#endif // __EXCEPTIONS_H__

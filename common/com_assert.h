#ifndef _COMMON_ASSERT_H_
#define _COMMON_ASSERT_H_

#include "com_debug.h"

#if _UART_DEBUG_PRINT_ && _ASSERT_PRINT_
    #define APP_ASSERT(expr)                                      \
    {                                                             \
        if(!(expr))                                               \
        {                                                         \
            DBG_MSG( "Assert failed: "#expr "[file: %s] [line: %d]\n ",   __FILE__, __LINE__);\
            while(1);                                             \
        }                                                         \
    }         
#else 
    #define APP_ASSERT(expr) 
#endif

#endif


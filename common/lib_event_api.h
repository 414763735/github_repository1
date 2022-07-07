/*************************************************************************
* lib_event_api.h
*************************************************************************/
#ifndef _LIB_EVENT_API_H_
#define _LIB_EVENT_API_H_

#include "lib_event_cfg.h"

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef uint8_t              LibBufSize_t;
typedef const LibEventId_t*  LibEventTbl_t;

/************************************************************************/
/*  Function Prototypes                                                 */
/************************************************************************/
Inst_t       LibEventCreate( const char* pNameStr, const LibEventId_t* pEventTbl, LibBufSize_t bufSize );
void         LibEventDelete( Inst_t inst );
LibEventId_t LibEventReceive( Inst_t inst );
void         LibEventSend( LibEventId_t eventId );
void         LibEventDoImmediately( LibEventId_t eventId );
#endif 

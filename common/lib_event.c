/*
	 发送者在发送EVENT时不需要指定接收者,对该事件的处理者只有一个
*/
/*************************************************************************
* lib_event.c
*************************************************************************/
#include "com_assert.h"

#include "lib_event_api.h"
#include "lib_pipe_api.h"
#include "lib_evt_api.h"
#include "lib_obj_api.h"

LIB_OBJ_MODULE_DEF(lib_event);

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef struct
{
    const LibEventId_t*   pEventTbl;   //!< pointer to table of supported events
    Inst_t                pipeInst;    //!< event queue 
} Obj_t;

/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/
bool_t sEventSupportedIs( Obj_t* pObj, LibEventId_t eventId );

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/
Inst_t LibEventCreate( const char* pNameStr, const LibEventId_t* pEventTbl, LibBufSize_t bufSize )
{
    Obj_t* pObj = NULL;

    APP_ASSERT(pNameStr);//
    APP_ASSERT(pEventTbl);//
    APP_ASSERT(bufSize);//

    /* For first instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {

    }

    /* Allocate memory for object */
    pObj = (Obj_t*)LIB_OBJ_CREATE(pNameStr, sizeof(Obj_t)); 
    APP_ASSERT(pObj);//

    /* Initialize instance */
    pObj->pEventTbl  = pEventTbl;
    pObj->pipeInst   = LibPipeCreate("Event", bufSize, sizeof(LibEventId_t));
    APP_ASSERT(pObj->pipeInst);//

    return OBJ2INST(pObj);
}

void LibEventDelete( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    /* Delete external resources */
    LibPipeDelete(pObj->pipeInst);

    /* De-allocate instantiation memory */
    LIB_OBJ_DELETE(pObj);

    /* Do for last deleted instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {

    }
}

LibEventId_t LibEventReceive( Inst_t inst )
{
    Obj_t*       pObj    = (Obj_t*)LIB_OBJ_PNTR(inst);
    LibEventId_t eventId = LibEvent_ID__NONE;

    if( LibPipeGetFillSize(pObj->pipeInst) )
    {
        /* Read KeyCode from local pipe */
        LibPipeRead(pObj->pipeInst, &eventId);
    }
    return eventId;
}
//基于消息队列，可以广播事件
void LibEventSend( LibEventId_t eventId )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_FIRST();

    /* Distribute event */
    while( pObj )
    {
        if( sEventSupportedIs(pObj, eventId) )//未注册该事件的OBJ无法接收该事件
        {
            if( LibPipeGetFreeSize(pObj->pipeInst) )    
            {
                /* Write Code into pipe */
                LibPipeWrite(pObj->pipeInst, &eventId);

                /* Send event to evt dispatcher */
                LibEvtSend(OBJ2INST(pObj), 0x1);
            }
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }
}

void LibEventDoImmediately( LibEventId_t eventId )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_FIRST();

    /* Distribute event */
    while( pObj )
    {
        if( sEventSupportedIs(pObj, eventId) )
        {
            if( LibPipeGetFreeSize(pObj->pipeInst) )    
            {
                /* Write Code into pipe */
                LibPipeWrite(pObj->pipeInst, &eventId);

                /* Send event to evt dispatcher and process till pipe is blank*/
								LibEvtSendDirect(OBJ2INST(pObj), 0x1);
            }
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
bool_t sEventSupportedIs( Obj_t* pObj, LibEventId_t eventId )
{
    const LibEventId_t* pEventTbl = pObj->pEventTbl;

    while( LibEvent_ID__NONE != *pEventTbl )
    {
        if( *pEventTbl == eventId )
            return true;

        pEventTbl++;
    }
    return false;
}

/** END of File *********************************************************/


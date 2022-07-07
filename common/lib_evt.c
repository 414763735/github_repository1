/*
	EVT在发送时，发送者需要指定接收者
*/
#include "com_assert.h"
#include <string.h>

#include "lib_evt_api.h"
#include "lib_seq_api.h"
#include "lib_obj_api.h"

LIB_OBJ_MODULE_DEF(lib_evt);

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef struct LibEvtInst_s
{
    Inst_t                txInst;      //!< pointer of transmitting instance
    LibEvtCbFunc_t        cbFunc;      //!< Function pointer to event callback
    Inst_t                rxInst;      //!< pointer to receiving instance
    uint32_t              flagsMsk;    //!< 32-bit Flag Mask register
    uint32_t              flagsEvt;    //!< 32-bit Flag Event register
    LibEvtPriority_t      priority;    //!< Priority (0-255) determines the position in timer table.
} Obj_t;

/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/
static void sPriorityUpdate( Obj_t* pObj );
static void sEventDispatcher( Inst_t parentInst );

/************************************************************************/
/*  Static Variables                                                    */
/************************************************************************/
static Inst_t sSeqInst = NULL;

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/
Inst_t LibEvtCreate( const char* pNameStr, Inst_t txInst, uint32_t flags, LibEvtCbFunc_t cbFunc, Inst_t rxInst, LibEvtPriority_t priority )
{
	//
    // 当使用 LibEvtSend 创建一个EVT时，需要带上txInst;  当调用回调函数时，需要带上rxInst。
    // 实际使用中会强调 txInst 的存在，但rxInst可以为空，也就是说回调函数不需要参数。
    // 创建时 txInst 存在的意义在于索引，遍历EVT链表时快速匹配找到对应EVT。这里不使用NameStr索引。
    //
    Obj_t* pObj = NULL;

    APP_ASSERT(txInst);//
    APP_ASSERT(flags);//
    APP_ASSERT(cbFunc);//

    /* For first instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {
        /* Configure high priority timer ( 0 msec.) */
        sSeqInst = LibSeqTimerCreate("evt", sEventDispatcher, NULL, LIB_SEQ_PRIOR__SYS_LIB_EVT);
        APP_ASSERT(sSeqInst);//
    }

    /* Allocate memory for object */
    pObj = (Obj_t*)LIB_OBJ_CREATE(pNameStr, sizeof(Obj_t));
    APP_ASSERT(pObj);//

    /* Initialize instance */
    pObj->txInst     = txInst;
    pObj->cbFunc     = cbFunc;
    pObj->rxInst     = rxInst;
    pObj->flagsMsk   = flags;
    pObj->flagsEvt   = LIB_EVT_FLAGS__NONE;
    pObj->priority   = priority;

    /* Repostion object in linked list based on priority */
    sPriorityUpdate(pObj);

    return OBJ2INST(pObj);
}

void LibEvtDelete( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    /* De-allocate instantiation memory */
    LIB_OBJ_DELETE(pObj);

    /* Do for last deleted instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {
        /* Configure high priority timer ( 0 msec.) */
        LibSeqTimerDelete(sSeqInst);
    }
}
//遍历所有对象，只有发送实例匹配的对象且该对象绑定了改Flags，才能接收该事件
void LibEvtSend( Inst_t txInst, uint32_t flags )
{
    Obj_t* pObj  = (Obj_t*)LIB_OBJ_FIRST();
    bool_t bEvt  = false;

    APP_ASSERT(txInst);//


    /* Search event list for matching events */
    while( pObj )
    {
        /* Both Instance-ID and event-ID must match, before we mark flag register */
        if( pObj->txInst == txInst )
        {
            uint32_t flagsReg = (pObj->flagsMsk) ? (pObj->flagsMsk & flags) : (1); /* For zero mask we set flag register to '1' */

            /* Critical section: should not be interrupted by 'sEventDispatcher' */
            {
                pObj->flagsEvt |= flagsReg;
            }

            bEvt = true;
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }

    if( bEvt )
    {
        /* Schedule 0 msec. timer to be processed by scheduler as soon as possible */
        LibSeqTimerStart(sSeqInst, 0, 0);
    }
}


//直接找到发送实例匹配的对象，并调用其回调函数进行处理
void LibEvtSendDirect( Inst_t txInst, uint32_t flags )
{
    Obj_t* pObj  = (Obj_t*)LIB_OBJ_FIRST();

    /* Search event list for matching events */
    while( pObj )
    {
        /* Both Instance-ID and event-ID must match, before we call callp-back function directly */
        if( pObj->txInst == txInst )
        {
            /* Call call-back function */
            pObj->cbFunc(pObj->rxInst, flags);
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }
}

void LibEvtPrioritySet( Inst_t inst, LibEvtPriority_t priority )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->priority = priority;
    sPriorityUpdate(pObj);
}

LibEvtPriority_t LibEvtPriorityGet( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return pObj->priority;
}

uint32_t LibEvtFlagsGet( Inst_t txInst)
{
    Obj_t   *pObj = (Obj_t*)LIB_OBJ_FIRST();
    uint32_t bEvt = 0;

    APP_ASSERT(txInst);//

    /* Search event list for matching events */
    while( pObj )
    {
        /* Both Instance-ID and event-ID must match, before we mark flag register */
        if( pObj->txInst == txInst )
        {
        	bEvt = pObj->flagsEvt;
        	break;
        }
        pObj = LIB_OBJ_NEXT(pObj);
    }
    return bEvt;
}

void LibEvtFlagsClean( Inst_t txInst)
{
    Obj_t   *pObj = (Obj_t*)LIB_OBJ_FIRST();

    APP_ASSERT(txInst);//

    /* Search event list for matching events */
    while( pObj )
    {
        /* Both Instance-ID and event-ID must match, before we mark flag register */
        if( pObj->txInst == txInst )
        {
        	pObj->flagsEvt = 0;
        	break;
        }
        pObj = LIB_OBJ_NEXT(pObj);
    }
    return;
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
//设置EVT列表上对象的优先级
static void sPriorityUpdate( Obj_t* pObj )
{
    Obj_t* pTmpObj = (Obj_t*)LIB_OBJ_FIRST();

    if( pTmpObj->priority > pObj->priority )
    {
        /* Move object to first place in list */
        LIB_OBJ_MOVE(NULL, pObj);
    }
    else
    {
        Obj_t* pPrvObj;

        do {
            pPrvObj = pTmpObj;

            /* Go to next timer instance */
            pTmpObj = LIB_OBJ_NEXT(pTmpObj);
        } while( (pTmpObj) && (pTmpObj->priority <= pObj->priority) );

        /* Move object to directly after pPrvObj */
        LIB_OBJ_MOVE(pPrvObj, pObj);
    }
}
//处理EVT时间列表上所有的注册的pobj对象,在调用LibEvtCreate时进行注册
static void sEventDispatcher( Inst_t parentInst )
{
    Obj_t* pObj  = (Obj_t*)LIB_OBJ_FIRST();

    /* parentInst is not used since blinkled application is implemented as a singleton */
    parentInst = parentInst; /* To avoid compiler warning */

    /* Search event list for timeouts */
    while( pObj )
    {
        uint32_t flagsEvt = pObj->flagsEvt; /* Double buffer flagReg value to be thread/ISR safe */;

        if( flagsEvt ) /* indicates whether event object requires to be handled */
        {
            /* Critical section: should not be interrupted by 'LibEvtSchedule' */
            {
                pObj->flagsEvt &= (~flagsEvt);
            }

            /* Call call-back function */
            if( pObj->flagsMsk )
                pObj->cbFunc(pObj->rxInst, flagsEvt);
            else
                pObj->cbFunc(pObj->rxInst, 0); /* because of zero mask we should also pass a zero flag register */

            break;
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }

    if( pObj )
    {
        /* re-schedule timer in case there are more pending events */
        LibSeqTimerStart(sSeqInst, 0, 0);
    }
}


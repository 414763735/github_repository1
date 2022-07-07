
#include "com_assert.h"
#include <string.h>

#include "lib_obj_api.h"
#include "lib_seq_api.h"
#include "platform_api.h"
#include "dev_adau1466.h"
#include "app_power.h"
#include "eqMsg.h"
#include "app_maual_eq.h"
#include "app_room_eq.h"

LIB_OBJ_MODULE_DEF(lib_seq);

/*
static const char*      sLibObjClassStr     = "lib_seq";
LibObjList_t            LibObjList_lib_seq  = NULL; //在调用OBJ create函数时分配的指向list的指针。
static LibObjList_t*    spLibObjList        = &LibObjList_lib_seq;//该变量使用来存指向list指针的指针，是固定下来，且不会变化的。

调用完LIB_OBJ_CREATE 函数，将初始化这个指针LibObjList_lib_seq，但多次在LibSeqTimerCreate中调用LIB_OBJ_CREATE
仍只会对LibObjList_lib_seq初始化一次，但会创建很多Header和对应的obj实体。

*/
/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define STOP_VALUE      ((LibSeqTimeMs_t)-1)

/************************************************************************/
/*  Inline Macros                                                       */
/************************************************************************/
#define DIAG_CONVERT_TO_FLOAT(pf, pi)  { *pf += (float)*pi; *pi  = 0; }

#define MILLI_SEC_DIFF(t1, t2)         ( ((t2) < (t1)) ? ((((LibSeqTimeMs_t)0)-1) - (t1) + (t2) + 1) : ((t2) - (t1)) )

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef struct
{
    LibSeqCbFunc_t        cbFunc;      //!< Function pointer to timer callback  定时器计数结束后回调函数指针。
    Inst_t                parentInst;  //!< Pointer to parent object            定时器计数结束后回调函数所在的OBJ的实例。
    LibSeqTimeMs_t        timeSet;     //!< Periodic time in msec. set by user  定时器周期需要重复的时间，单位毫秒。
    LibSeqPriority_t      priority;    //!< Priority (0-255) determines the position in timer table. 定时器优先级。0 is the highest.
    bool                  bRealTime;   //!< Preserve realtime constratin        在该项目中实际上未使用。

    LibSeqTimeMs_t        timePrv;     //!< Previous time sample                上一次计数时的系统时间值。用于和现在的系统时间比较算出经过时间
    LibSeqTimeMs_t        timeRun;     //!< Remaining time left in msec.        和timePrv联合使用，每次减去经过时间。当为0时即超时，调用回调函数。
    
} Obj_t;

/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/
static void sSequencerSleep(uint32_t time);
static void sPriorityUpdate( Obj_t* pObj );
static LibSeqTimeMs_t sSequencerHandler(void);

/************************************************************************/
/*  Static Variables                                                    */
/************************************************************************/
static bool             sbWake       = true;// the sbWake is init with ture state, and will not be changed if don't use UFD and other console
static LibSeqTimeMs_t   sOverRunTime = 0;
static bool             sbExitNot    = true;
static bool             seqSleep     = true;

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/
void LibSeqSetSuspendMode( bool isTrue )
{
    sbWake = !isTrue;
}

bool LibSeqIsSuspendMode( void )
{
    return !sbWake;
}

//*****************************************************************************
//  LibSeqTimerCreate
//! 创建一个轮询(SeqTimerObj)，并加入Sequencer list，将在sSequencerHandler函数内轮询检查超时。
//! 轮询时间到了，调用回调函数。
//!
//! \param * pNameStr  实例的名字指针，指向实例名字。如"gpio"。
//! \param * cbFunc    指向实例中回调函数的指针。
//! \param * parentInst Pointer reference to parent object
//!              指向实例自身的指针。
//! \param   priority  (0..255) 0=highest, 255=lowest
//!              该实例的优先级，优先级高的函数轮询时优先执行。也就是它处于轮询列表靠前的位置。0最高。
//! 
//! \return  Instance 
//!              将创建的轮询实例(SeqTimerObj)的头指针作为返回值返回。
//
//*****************************************************************************
Inst_t LibSeqTimerCreate( const char* pNameStr, LibSeqCbFunc_t cbFunc, Inst_t parentInst, LibSeqPriority_t priority )
{
    Obj_t* pObj = NULL;

    APP_ASSERT(cbFunc);//

    /* Do for first created instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {

    }

    /* Allocate memory for object */
    pObj = (Obj_t*)LIB_OBJ_CREATE(pNameStr, sizeof(Obj_t));
    APP_ASSERT(pObj);//

    /* Initialize instance */
    pObj->cbFunc      = cbFunc;
    pObj->parentInst  = parentInst;
    pObj->timeSet     = 0;
    pObj->priority    = priority;
    pObj->bRealTime   = false;
    pObj->timePrv     = 0;
    pObj->timeRun     = STOP_VALUE;

    /* Repostion object in linked list based on priority */
    sPriorityUpdate(pObj);

    LIB_OBJ_LOGTYPE(pObj, LIB_OBJ_LOG_TYPE__WARNING, true);

    return OBJ2INST(pObj);
}

void LibSeqTimerDelete( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    /* De-allocate instantiation memory */
    LIB_OBJ_DELETE(pObj);

    /* Do for last deleted instantiation only */
    if( LIB_OBJ_ZEROINST() )
    {

    }
}

//-------------------------------------------------------------------------------------------------
//! @brief      Start timer.
//!
//! @param[in]  inst     Reference to instance
//! @param[in]  timeMs   Timer interval between this Start function and next call-back function.

//!                 不断递减到0时，判定超时并调用回调函数。
//! @param[in]  periodMs Time interval of periodic timer (use '0' if one-shot timer)
//!                 周期值，为0时，调用一次回调函数后stop，非0时，调用函数后重置时间为该周期值。
//-------------------------------------------------------------------------------------------------
void LibSeqTimerStart( Inst_t inst, LibSeqTimeMs_t timeMs, LibSeqTimeMs_t periodMs )
{
    Obj_t*         pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->timePrv = (LibSeqTimeMs_t)HAL_GetCurrentMsCount();
    pObj->timeRun = timeMs;
    pObj->timeSet = periodMs;
}

void LibSeqTimerStop( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->timeRun = STOP_VALUE;
}

bool LibSeqTimerRunningIs( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return (STOP_VALUE == pObj->timeRun) ? (true) : (false);
}

void LibSeqTimerPeriodSet( Inst_t inst, LibSeqTimeMs_t periodMs )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->timeSet = periodMs;
}

LibSeqTimeMs_t LibSeqTimerPeriodGet( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return pObj->timeSet;
}

void LibSeqTimerPrioritySet( Inst_t inst, LibSeqPriority_t priority )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->priority = priority;
    sPriorityUpdate(pObj);
}

LibSeqPriority_t LibSeqTimerPriorityGet( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return pObj->priority;
}

void LibSeqTimerRealTimeSet( Inst_t inst, bool bIsTrue )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->bRealTime = bIsTrue;
}

bool LibSeqTimerRealTimeIs( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return pObj->bRealTime;
}

LibSeqTimeMs_t LibSeqTimerOverRunTimeGet( void )
{
    return sOverRunTime;
}

void LibSeqAbort( void )
{
    sbExitNot = false;
}

void LibSeqWakeup(void)
{
    seqSleep = false;
}

void LibSeqTask( void )
{
    static LibSeqTimeMs_t  suspendTimeMs;
    
    do
    {
        suspendTimeMs = 0;
        /* Handle sequencer until no timers are expired */
        while ( !suspendTimeMs )
        {
            suspendTimeMs = sSequencerHandler();
        }
        if (AppPowerStatus_IsInWorkingMode())
        {
            //AppHdmi_UpdateServ();
            ManualEqTask();
			RoomEqTask();
        }
        else
        {
            seqSleep = true;
            sSequencerSleep(suspendTimeMs);
        }
    }while(sbExitNot);
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
static void sSequencerSleep(uint32_t time)
{
    uint32_t elapsed,target,current;

    current = HAL_GetCurrentMsCount();
    target = current + time;

    do{
        if (current >= target)
        {
            elapsed = current - target;
            if (elapsed < 0xF0000000L)
            {
                break;
            }
        }
        current = HAL_GetCurrentMsCount();
    }while (seqSleep);
}

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

//*****************************************************************************
//  sSequencerHandler
//! 任务调度的主循环函数。 搜索Sequencer list，即由 LibSeqTimerCreate 函数创建的元的列表。
//! 函数内主循环每一次循环处理一个元，处理时元中记录的时间先减去经历时间，当得出超时时，调用回调函数。
//! 当处理一个回调函数后，退出主循环。
//!
//! 
//! \return  当到遍历整个列表后，pObj的指针会清空，所以返回timeWakeUp，在该函数外去等待。
//!          timeWakeUp记录了最快要超时的剩余时间。
//
//*****************************************************************************
static LibSeqTimeMs_t sSequencerHandler ( void )
{
    Obj_t*          pObj       = (Obj_t*)LIB_OBJ_FIRST();
    LibSeqTimeMs_t  timeNew    = (LibSeqTimeMs_t)HAL_GetCurrentMsCount();
    LibSeqTimeMs_t  timeWakeUp = STOP_VALUE;

    /* Search timer list for timeouts */
    //搜索Sequencer list，即由 LibSeqTimerCreate 函数创建的元的列表。
    while ( pObj )
    {

        /* All timers with priority greater than zero will be suspended if sbWake=false */
        /* the sbWake is init with ture state, and will not be changed if don't use UFD and other console*/
        if( (0 < pObj->priority) && !sbWake )
        {
            break;
        }

        if( STOP_VALUE > pObj->timeRun )
        {
            LibSeqTimeMs_t timeDiff = MILLI_SEC_DIFF(pObj->timePrv, timeNew);

            /* Update previous time */
            pObj->timePrv = timeNew;

            if( pObj->timeRun > timeDiff )
            {
                /* Update Running Time */
                pObj->timeRun -= timeDiff;

                if( pObj->timeRun < timeWakeUp )
                {
                    //
                    timeWakeUp = pObj->timeRun;
                }
            }
            else /* timeout */
            {
                LibSeqTimeMs_t timeTmp;

                /* Load running timer with over run time (latency of current time-out), which maybe queried only from Call Back function */
                sOverRunTime = timeDiff - pObj->timeRun;

                /* Reload or Stop timer */
                timeTmp = pObj->timeRun = (pObj->timeSet) ? (pObj->timeSet) : (STOP_VALUE);

                /* Call call-back function */
                pObj->cbFunc(pObj->parentInst);

                /* Apply Real-Time mode for periodic timer: re-adjust new timer based on previous timer over run time */
                if( pObj->bRealTime )
                {
                    if( (STOP_VALUE != pObj->timeRun) && (timeTmp == pObj->timeRun) )
                    {
                        if( pObj->timeRun < sOverRunTime )
                        {
                            LIB_OBJ_LOG(pObj, LIB_OBJ_LOG_TYPE__WARNING, "Real-Time violation by: %d msec!", sOverRunTime - pObj->timeRun);

                            /* Full over run compensation is not possible */
                            pObj->timeRun = 0;
                        }
                        else
                        {
                            /* Compensate for timer over run time */
                            pObj->timeRun -= sOverRunTime;
                        }
                    }
                }
                sOverRunTime = 0;
                
                /* use break to process only 1 timer at the time */
                break; 
            }
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT(pObj);
    }
    
    return (pObj) ? ((LibSeqTimeMs_t)0) : (timeWakeUp);
}

/** END of File *********************************************************/


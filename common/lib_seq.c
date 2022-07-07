
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
LibObjList_t            LibObjList_lib_seq  = NULL; //�ڵ���OBJ create����ʱ�����ָ��list��ָ�롣
static LibObjList_t*    spLibObjList        = &LibObjList_lib_seq;//�ñ���ʹ������ָ��listָ���ָ�룬�ǹ̶��������Ҳ���仯�ġ�

������LIB_OBJ_CREATE ����������ʼ�����ָ��LibObjList_lib_seq���������LibSeqTimerCreate�е���LIB_OBJ_CREATE
��ֻ���LibObjList_lib_seq��ʼ��һ�Σ����ᴴ���ܶ�Header�Ͷ�Ӧ��objʵ�塣

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
    LibSeqCbFunc_t        cbFunc;      //!< Function pointer to timer callback  ��ʱ������������ص�����ָ�롣
    Inst_t                parentInst;  //!< Pointer to parent object            ��ʱ������������ص��������ڵ�OBJ��ʵ����
    LibSeqTimeMs_t        timeSet;     //!< Periodic time in msec. set by user  ��ʱ��������Ҫ�ظ���ʱ�䣬��λ���롣
    LibSeqPriority_t      priority;    //!< Priority (0-255) determines the position in timer table. ��ʱ�����ȼ���0 is the highest.
    bool                  bRealTime;   //!< Preserve realtime constratin        �ڸ���Ŀ��ʵ����δʹ�á�

    LibSeqTimeMs_t        timePrv;     //!< Previous time sample                ��һ�μ���ʱ��ϵͳʱ��ֵ�����ں����ڵ�ϵͳʱ��Ƚ��������ʱ��
    LibSeqTimeMs_t        timeRun;     //!< Remaining time left in msec.        ��timePrv����ʹ�ã�ÿ�μ�ȥ����ʱ�䡣��Ϊ0ʱ����ʱ�����ûص�������
    
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
//! ����һ����ѯ(SeqTimerObj)��������Sequencer list������sSequencerHandler��������ѯ��鳬ʱ��
//! ��ѯʱ�䵽�ˣ����ûص�������
//!
//! \param * pNameStr  ʵ��������ָ�룬ָ��ʵ�����֡���"gpio"��
//! \param * cbFunc    ָ��ʵ���лص�������ָ�롣
//! \param * parentInst Pointer reference to parent object
//!              ָ��ʵ�������ָ�롣
//! \param   priority  (0..255) 0=highest, 255=lowest
//!              ��ʵ�������ȼ������ȼ��ߵĺ�����ѯʱ����ִ�С�Ҳ������������ѯ�б�ǰ��λ�á�0��ߡ�
//! 
//! \return  Instance 
//!              ����������ѯʵ��(SeqTimerObj)��ͷָ����Ϊ����ֵ���ء�
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

//!                 ���ϵݼ���0ʱ���ж���ʱ�����ûص�������
//! @param[in]  periodMs Time interval of periodic timer (use '0' if one-shot timer)
//!                 ����ֵ��Ϊ0ʱ������һ�λص�������stop����0ʱ�����ú���������ʱ��Ϊ������ֵ��
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
//! ������ȵ���ѭ�������� ����Sequencer list������ LibSeqTimerCreate ����������Ԫ���б�
//! ��������ѭ��ÿһ��ѭ������һ��Ԫ������ʱԪ�м�¼��ʱ���ȼ�ȥ����ʱ�䣬���ó���ʱʱ�����ûص�������
//! ������һ���ص��������˳���ѭ����
//!
//! 
//! \return  �������������б��pObj��ָ�����գ����Է���timeWakeUp���ڸú�����ȥ�ȴ���
//!          timeWakeUp��¼�����Ҫ��ʱ��ʣ��ʱ�䡣
//
//*****************************************************************************
static LibSeqTimeMs_t sSequencerHandler ( void )
{
    Obj_t*          pObj       = (Obj_t*)LIB_OBJ_FIRST();
    LibSeqTimeMs_t  timeNew    = (LibSeqTimeMs_t)HAL_GetCurrentMsCount();
    LibSeqTimeMs_t  timeWakeUp = STOP_VALUE;

    /* Search timer list for timeouts */
    //����Sequencer list������ LibSeqTimerCreate ����������Ԫ���б�
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


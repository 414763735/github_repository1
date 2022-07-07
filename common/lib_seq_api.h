
#ifndef _LIB_SEQ_API_H_
#define _LIB_SEQ_API_H_

#include "hal_system_timer.h"
#include "lib_seq_cfg.h"

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/


/************************************************************************/
/*  Inline Macros                                                       */
/************************************************************************/

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef uint32_t LibSeqTimeMs_t;
typedef uint8_t  LibSeqPriority_t;

typedef void (*LibSeqCbFunc_t)( Inst_t parentInst );

/************************************************************************/
/*  Function Prototypes                                                 */
/************************************************************************/
//-------------------------------------------------------------------------------------------------
//! @brief      Scheduler suspend/resume control.
//!
//! @param[in]  isTrue
//!             - true  Enable suspend mode
//!             - false Resume scheduler
//-------------------------------------------------------------------------------------------------
void LibSeqSetSuspendMode( bool isTrue );

//-------------------------------------------------------------------------------------------------
//! @brief      Suspend suspend/resume status.
//!
//! @retval     true  Scheduler is suspended
//! @retval     false Scheduler is active
//-------------------------------------------------------------------------------------------------
bool LibSeqIsSuspendMode( void );

//-------------------------------------------------------------------------------------------------
//! @brief      Create timer handler.
//!
//! @param[in]  pNameStr    Name of timer
//! @param[in]  cbFunc      Function pointer of local handler function
//! @param[in]  parentInst  Pointer reference to parent object
//! @param[in]  priority    Priority (0..255) 0=highest, 255=lowest
//!
//! @return     Instance
//-------------------------------------------------------------------------------------------------
Inst_t LibSeqTimerCreate( const char* pNameStr, LibSeqCbFunc_t cbFunc, Inst_t parentInst, LibSeqPriority_t priority );

//-------------------------------------------------------------------------------------------------
//! @brief      Destroy timer handler.
//!
//! @param[in]  inst : Reference to instance
//-------------------------------------------------------------------------------------------------
void LibSeqTimerDelete( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Start timer.
//!
//! @param[in]  inst     Reference to instance
//! @param[in]  timeMs   Timer interval between this Start function and next call-back function.
//! @param[in]  periodMs Time interval of periodic timer (use '0' if one-shot timer)
//-------------------------------------------------------------------------------------------------
void LibSeqTimerStart( Inst_t inst, LibSeqTimeMs_t timeMs, LibSeqTimeMs_t periodMs );

//-------------------------------------------------------------------------------------------------
//! @brief      Stop timer.
//!
//! @param[in]  inst Reference to instance
//-------------------------------------------------------------------------------------------------
void LibSeqTimerStop( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Stop all timers.
//-------------------------------------------------------------------------------------------------
void LibSeqTimerStopAll( void );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer running status.
//!
//! @param[in]  inst   Reference to instance
//!
//! @retval     true  Timer is running
//! @retval     false Timer is stopped
//-------------------------------------------------------------------------------------------------
bool LibSeqTimerRunningIs( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Periodic time control.
//!
//! @param[in]  inst     Reference to instance
//! @param[in]  periodMs Periodic time in milliseconds
//-------------------------------------------------------------------------------------------------
void LibSeqTimerPeriodSet( Inst_t inst, LibSeqTimeMs_t periodMs );

//-------------------------------------------------------------------------------------------------
//! @brief      Periodic time status.
//!
//! @param[in]  inst  Reference to instance
//!
//! @return     Periodic time in milliseconds
//-------------------------------------------------------------------------------------------------
LibSeqTimeMs_t LibSeqTimerPeriodGet( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer priority control.
//!
//! @param[in]  inst       Reference to instance
//! @param[in]  priority   0..255 (0=highest,255=lowest)
//-------------------------------------------------------------------------------------------------
void LibSeqTimerPrioritySet( Inst_t inst, LibSeqPriority_t priority );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer priority status.
//!
//! @param[in]  inst Reference to instance
//!
//! @return     Priority (0=highest....255=lowest).
//-------------------------------------------------------------------------------------------------
LibSeqPriority_t LibSeqTimerPriorityGet( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer real-time control.
//!
//!             If enabled and timer is used in repetitive mode the scheduler will preserve the
//!             the timer expiration frequency despite if timer handling is delayed by other timer
//!             handlers.
//!
//! @param[in]  inst     Reference to instance
//! @param[in]  bIsTrue  \c true, \c false
//-------------------------------------------------------------------------------------------------
void LibSeqTimerRealTimeSet( Inst_t inst, bool bIsTrue );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer real-time status.
//!
//!             If enabled and timer is used in repetitive mode the scheduler will preserve the
//!             the timer expiration frequency despite if timer handling is delayed by other timer
//!             handlers.
//!
//! @param[in]  inst    Reference to instance
//!
//! @retval     true   Real-time mode is enabled
//! @retval     false  Real-time mode is disabled
//-------------------------------------------------------------------------------------------------
bool LibSeqTimerRealTimeIs( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Provides over run time. Can only be called from timer call back function.
//!
//! @return     Overrun time in msec.
//-------------------------------------------------------------------------------------------------
LibSeqTimeMs_t LibSeqTimerOverRunTimeGet( void );

//-------------------------------------------------------------------------------------------------
//! @brief      Abort sequencer task.
//!
//!             This function exits the sequencer task.
//-------------------------------------------------------------------------------------------------
void LibSeqAbort( void );

void LibSeqWakeup(void);

//-------------------------------------------------------------------------------------------------
//! @brief      Sequencer task.
//!
//! @param[in]  threadSafeSemaId : Semaphore for the critical section ID, now we don't support.
//!
//!             Starts framework operations.
//!             threadSafeSemaId can be (0) for one thread systems.
//!             For multi-thread systems threadSafeSemaId is a pointer to a semaphore
//!             that is used in a critical sections wrapping functions called from
//!             other threads.
//!             This function won't exit.
//-------------------------------------------------------------------------------------------------
void LibSeqTask( void );

#endif


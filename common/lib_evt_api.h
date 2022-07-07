
#ifndef LIB_EVT_API_H
#define LIB_EVT_API_H

#include "lib_evt_cfg.h"

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define LIB_EVT_FLAGS__NONE          ((uint32_t)0x00000000)
#define LIB_EVT_FLAGS(bit)           ((uint32_t)(1<<bit))

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef uint16_t LibEvtTimeMs_t;
typedef uint8_t LibEvtPriority_t;

typedef void (*LibEvtCbFunc_t)( Inst_t RxInst, uint32_t flags );

/************************************************************************/
/*  Function Prototypes                                                 */
/************************************************************************/
//-------------------------------------------------------------------------------------------------
//! @brief      Creates receiving event object.
//!
//! @param[in]  pNameStr  Name of event
//! @param[in]  txInst    Reference of module-instatiation that schedules (sends) event
//! @param[in]  flags     Indication of flags that apply to this notification call back.
//! @param[in]  cbFunc    Function pointer of local handler function
//! @param[in]  rxInst    Reference of module-instantiation that receives this event
//! @param[in]  priority  Priority (0..255) 0=highest, 255=lowest
//!
//! @return     Instance
//-------------------------------------------------------------------------------------------------
Inst_t LibEvtCreate( const char* pNameStr, Inst_t txInst, uint32_t flags, LibEvtCbFunc_t cbFunc, Inst_t rxInst, LibEvtPriority_t priority );

//-------------------------------------------------------------------------------------------------
//! @brief      Deletes receiving event object.
//!
//! @param[in]  inst Instance of requested event
//-------------------------------------------------------------------------------------------------
void LibEvtDelete( Inst_t inst );

//-------------------------------------------------------------------------------------------------
//! @brief      Schedule event.
//!
//! @param[in]  txInst   Reference of module-instantiation that schedules this event
//! @param[in]  flags    32-bit flag register that can be used to communicate multiple events.
//-------------------------------------------------------------------------------------------------
void LibEvtSend( Inst_t txInst, uint32_t flags );
void LibEvtSendDirect( Inst_t txInst, uint32_t flags );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer priority control.
//!
//! @param[in]  inst      Reference to instance
//! @param[in]  priority  0..255 (0=highest,255=lowest).
//-------------------------------------------------------------------------------------------------
void LibEvtPrioritySet( Inst_t inst, LibEvtPriority_t priority );

//-------------------------------------------------------------------------------------------------
//! @brief      Timer priority status.
//!
//! @param[in]  inst      Reference to instance
//!
//! @return     Priority (0=highest....255=lowest).
//-------------------------------------------------------------------------------------------------
LibEvtPriority_t LibEvtPriorityGet( Inst_t inst );

uint32_t LibEvtFlagsGet(Inst_t inst);

void LibEvtFlagsClean( Inst_t inst);

#endif // LIB_EVT_API_H


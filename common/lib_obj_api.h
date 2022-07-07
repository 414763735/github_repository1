#ifndef _LIB_OBJ_API_H_
#define _LIB_OBJ_API_H_


/************************************************************************/
/*  MACROS                                                              */
/************************************************************************/
#if _UART_DEBUG_PRINT_ && _ASSERT_PRINT_
	#define OBJ_DEBUG_ASSERT( expr ) \
        ( (void)( (/*lint -e{506}*/!(expr)) ? DBG_MSG( "assert:"#expr "[f:%s] [l:%d]\n ",   __FILE__, __LINE__),((void)1) : ((void)0) ) )
#else
    #define OBJ_DEBUG_ASSERT( expr ) \
        ( (void)0 )
#endif
/* Construct to stringify defines */
#define OBJ_STRINGIFY(x)       #x
#define OBJ_DEF2STR(x)         OBJ_STRINGIFY(x)

/* local look-up definition */
#define LIB_OBJ_LIST         (*spLibObjList)

//-------------------------------------------------------------------------------------------------
//! @brief      'LIB_OBJ_MODULE_DEF()' must be defined whenever 'LIB_OBJ_CREATE()' is used.
//!
//!             When a module object is shared between multiple source (.c) files then the source file
//!             that containins the create/delete methods should be using 'LIB_OBJ_MODULE_DEF(str)'
//!             while all other source files that refer to the same module object should use
//!             'LIB_OBJ_MODULE_EXT(str)'.
//!             Both macros should only be used outside of functions.
//!
//! @param[in]  str  Pointer to string containing name of module.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_MODULE_DEF(str)        static const char* sLibObjClassStr = OBJ_DEF2STR(str); \
									   LibObjList_t LibObjList_##str = NULL; \
									   static LibObjList_t* spLibObjList = &LibObjList_##str
									   
#define LIB_OBJ_MODULE_EXT(str)        extern LibObjList_t LibObjList_##str; \
									   static LibObjList_t* spLibObjList = &LibObjList_##str

//-------------------------------------------------------------------------------------------------
//! @brief      Allocates memory needed for the instantiation defined in module.
//!
//!             All creations by the same module are linked together through a linked list.
//!
//! @param[in]  str   Pointer to string containing name of instantiation.
//! @param[in]  size  Size of instantiation as defined in module.
//!
//! @return     pointer to allocated instantiation object
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_CREATE(str, size)      LibObjCreate(&LIB_OBJ_LIST, sLibObjClassStr, str, size)

//-------------------------------------------------------------------------------------------------
//! @brief      Frees instantiation from dynamic memory pool.
//!
//! @param[in]  p  Pointer to instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_DELETE(p)              LibObjDelete(p); p=0

//-------------------------------------------------------------------------------------------------
//! @brief      Change internal state to 'Started'.
//!
//! @param[in]  p  Pointer to instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_START(p)               LibObjStart(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Change internal state to 'Stopped'.
//!
//! @param[in]  p  Pointer to instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_STOP(p)                LibObjStop(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Change internal state to 'Stopped'.
//!
//! @param[in]  p  Pointer to instantiation object.
//!
//! @return     Boolean
//! @retval     false  Stopped
//! @retval     true   Started
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_STARTED_IS(p)          LibObjStartedIs(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Returns pointer to module (class) name associated to instantiation.
//!
//! @param[in]  p  Pointer to instantiation object.
//!
//! @return     Pointer to string containing name of module.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_NAME_MODULE(p)         LibObjNameModuleGet(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Returns pointer to instance name associated to instantiation.
//!
//! @param[in]  p  Pointer to instantiation object.
//!
//! @return     Pointer to string containing name of instantiation.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_NAME_INSTANCE(p)       LibObjNameInstanceGet(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Converts external instantiation reference to pointer of instantiation object as
//!             defined locally in module. Creates an assertion if user would provide a reference
//!             to a different class of instantiation.
//!
//! @param[in]  inst  External reference to instantiation.
//!
//! @return     Pointer to instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_PNTR(inst)             ( OBJ_DEBUG_ASSERT(LibObjCheck(&LIB_OBJ_LIST, INST2OBJ(inst))), (INST2OBJ(inst)) )

//-------------------------------------------------------------------------------------------------
//! @brief      Provides a pointer to the first listed instantiation in the linked list.
//!             A NULL value is returned if no instantiations exist.
//!
//! @return     Pointer to first instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_FIRST()                LibObjFirstGet(LIB_OBJ_LIST)

//-------------------------------------------------------------------------------------------------
//! @brief      Provides a pointer to the next listed instantiation in the linked list based on
//!             a provided pointer to a different instantiation object of the same class.
//!             A NULL value is returned if referred to last instantiation linked list.
//!
//! @param[in]  p  Pointer to instantiation object.
//!
//! @return     Pointer to next instantiation object.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_NEXT(p)                LibObjNextGet(p)

//-------------------------------------------------------------------------------------------------
//! @brief      Returns pointer of object by name as used when object was created.
//!
//! @return     Pointer to object with matched name or NULL if no objects found.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_PNTR_BY_NAME(str)      LibObjPointerGetByName(LIB_OBJ_LIST, str)

//-------------------------------------------------------------------------------------------------
//! @brief      Relocates an instantiation ('psrc') to a different position in linked list.
//!             The insertion point for 'psrc' is directly after the 'pdes' instance.
//!
//! @param[in]  pdes  Pointer to instantiation object used as insertion point in linked list.
//!                   pdes may also be assigned with 'NULL'. In this case 'psrc' will be inserted
//!                   directly in front of the first listed instance and becomes now the new
//!                   first instance in linked-list.
//!
//! @param[in]  psrc  Pointer to instantiation object that needs to be relocated.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_MOVE(pdes, psrc)       LibObjMove(pdes, psrc)

//-------------------------------------------------------------------------------------------------
//! @brief      Check if module is currently not been instantiated.
//!
//! @retval     true   If zero instantiations exist.
//! @retval     false  If 1 or more instantiations exist.
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_ZEROINST()             ( (LIB_OBJ_LIST) ? (false) : (true) )

//-------------------------------------------------------------------------------------------------
//! @brief      Logging (printf) command that starts of with time stamp, module name and instance name
//!             e.g.: 0.110-si_drv_frmdet.HDMI-RX   : `<printf string>`
//!
//! @param[in]  p         Pointer to instantiation object.
//! @param[in]  logtype   Print channel.
//! @param[in]  ... String and parameters.
//-------------------------------------------------------------------------------------------------
//todo: this is only a fast way to disable the debug print, but we need make it in a normal way.
#define LIB_OBJ_LOG(p, logtype, ...)   //LibObjLog(p, logtype, __VA_ARGS__)  

//-------------------------------------------------------------------------------------------------
//! @brief      Enables/Disables a log type
//!
//! @param[in]  p    Pointer to instantiation object.
//! @param[in]  type Type of logging (see LibObjLogType_t).
//! @param[in]  b    \c false to disable, or \c true to enable
//-------------------------------------------------------------------------------------------------
#define LIB_OBJ_LOGTYPE(p, type, b)    //LibObjLogTypeSet(p, type, b)

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef void*    LibObjList_t;
typedef uint16_t LibObjSize_t;

typedef enum
{
    LIB_OBJ_LOG_TYPE__INFO,
    LIB_OBJ_LOG_TYPE__WARNING,
    LIB_OBJ_LOG_TYPE__ERROR,
    LIB_OBJ_LOG_TYPE__DEBUG
} LibObjLogType_t;

/************************************************************************/
/*  Function Prototype                                                  */
/*  Note: Please don't directly refer to following methods. Use Macros  */
/*        only.                                                         */
/************************************************************************/
void*  LibObjCreate( LibObjList_t* pInstList, const char* pClassStr, const char* pInstStr, LibObjSize_t size );
void   LibObjDelete( void* p );
void   LibObjStart( void* p );
void   LibObjStop( void* p );
bool   LibObjStartedIs( void* p );
const char* LibObjNameModuleGet( void* p );
const char* LibObjNameInstanceGet( void* p );
bool   LibObjCheck( LibObjList_t* pInstList, void* p );
void*  LibObjFirstGet( LibObjList_t instList );
void*  LibObjNextGet( void* p );
void   LibObjMove( void* pDes, void* pSrc );
void*  LibObjPointerGetByName( LibObjList_t instList, const char* pClassStr );
void   LibObjLog( void* p, LibObjLogType_t logType, const char* pFrm, ... );
void   LibObjLogTypeSet( void* p, LibObjLogType_t logType, bool isTrue );



#endif


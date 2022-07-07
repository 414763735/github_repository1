/*************************************************************************
* lib_pipe.c
*************************************************************************/
#include <string.h>

#include "com_assert.h"

#include "lib_pipe_api.h"
#include "lib_obj_api.h"
#include "lib_malloc_api.h"

LIB_OBJ_MODULE_DEF(lib_pipe);

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef struct
{
    uint8_t*            pMem;        //!< Pointer to memory pool object
    uint8_t             objSize;     //!< Size of memory pool in bytes
    uint16_t            volume;      //!< Pipe depth
    uint16_t            rdPtr;       //!< Read Pointer
    uint16_t            wrPtr;       //!< Write Pointer
    bool_t              bNewData;    //!< New data indicator
} Obj_t;

/************************************************************************/ 
/*  Local Prototypes                                                    */
/************************************************************************/
static uint16_t sGetFillSize( Obj_t* pObj );
static uint16_t sGetFreeSize( Obj_t* pObj );

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/
Inst_t LibPipeCreate( const char* pNameStr, uint16_t volume, uint8_t objSize )
{
    Obj_t* pObj = NULL;

    APP_ASSERT(pNameStr);//

    /* Action for first created instantiation */
    if( LIB_OBJ_ZEROINST() )
    {
    }

    /* Request memory for object */
    pObj = (Obj_t*)LIB_OBJ_CREATE(pNameStr, sizeof(Obj_t)); 
    APP_ASSERT(pObj);//

    /* Initialize Instance */
    pObj->pMem       = (uint8_t*)LibMallocCreate(objSize*volume);
    pObj->objSize    = objSize;
    pObj->volume     = volume; 
    pObj->rdPtr      = 0;
    pObj->wrPtr      = 0;
    pObj->bNewData   = false;

    return OBJ2INST(pObj);
}

void LibPipeDelete( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    pObj->rdPtr      = 0;
    pObj->wrPtr      = 0;
    LibMallocDelete(pObj->pMem);

    /* Delete object out of link list */
    LIB_OBJ_DELETE(pObj);

    /* Action for last delete instantiation */
    if( LIB_OBJ_ZEROINST() )
    {
    }
}

LibPipeError_t LibPipeWrite( Inst_t inst, void* pData )
{
    Obj_t*   pObj = (Obj_t*)LIB_OBJ_PNTR(inst);
    uint8_t* pSrc = (uint8_t*)pData;
    uint8_t* pDes = (uint8_t*)(pObj->pMem + pObj->wrPtr*pObj->objSize);

    /* Check for full buffer */
    if( 0 == sGetFreeSize(pObj) )
        return LIB_PIPE_ERROR_FULL;

    /* Copy data object into pipe */
    memcpy(pDes, pSrc, pObj->objSize);

    /* Set event flag any time when current pipe is empte */
    if( pObj->rdPtr == pObj->wrPtr )
        pObj->bNewData = true;

    /* Increment Write pointer */
    if( pObj->volume > pObj->wrPtr )
        pObj->wrPtr++;
    else
        pObj->wrPtr = 0;

    return LIB_PIPE_ERROR_NONE;
}

LibPipeError_t LibPipeRead( Inst_t inst, void* pData )
{
    Obj_t*   pObj = (Obj_t*)LIB_OBJ_PNTR(inst);
    uint8_t* pSrc = (uint8_t*)(pObj->pMem + pObj->rdPtr*pObj->objSize);
    uint8_t* pDes = (uint8_t*)pData;

    /* Check for empty buffer */
    if( 0 == sGetFillSize(pObj) )
        return LIB_PIPE_ERROR_EMPTY;

    /* Read data out of pipe */
    memcpy(pDes, pSrc, pObj->objSize);

    /* Increment Write pointer */
    if( pObj->volume > pObj->rdPtr )
        pObj->rdPtr++;
    else
        pObj->rdPtr = 0;

    return LIB_PIPE_ERROR_NONE;
}

LibPipeError_t LibPipeCheck( Inst_t inst, void* pData )
{
    Obj_t*   pObj = (Obj_t*)LIB_OBJ_PNTR(inst);
    uint8_t* pSrc = (uint8_t*)(pObj->pMem + pObj->rdPtr*pObj->objSize);
    uint8_t* pDes = (uint8_t*)pData;

    /* Check for empty buffer */
    if( 0 == sGetFillSize(pObj) )
        return LIB_PIPE_ERROR_EMPTY;

    /* Read data out of pipe */
    memcpy(pDes, pSrc, pObj->objSize);
    return LIB_PIPE_ERROR_NONE;
}

uint16_t LibPipeGetFillSize( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return sGetFillSize(pObj);
}

uint16_t LibPipeGetFreeSize( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return sGetFreeSize(pObj);
}

uint16_t LibPipeGetTotalSize( Inst_t inst )
{
    Obj_t* pObj = (Obj_t*)LIB_OBJ_PNTR(inst);

    return pObj->volume;
}

bool_t LibPipeNewDataIs( Inst_t inst )
{
    Obj_t* pObj     = (Obj_t*)LIB_OBJ_PNTR(inst);
    bool_t bNewData = pObj->bNewData;

    bNewData = pObj->bNewData;
    if( bNewData )
    {
        pObj->bNewData = false;
    }

    return bNewData;
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
static uint16_t sGetFillSize( Obj_t* pObj )
{
    uint16_t wrPtr = pObj->wrPtr;
    uint16_t rdPtr = pObj->rdPtr;

    return (wrPtr<rdPtr) ? (pObj->volume - rdPtr + wrPtr + 1) : (wrPtr-rdPtr);
}

static uint16_t sGetFreeSize( Obj_t* pObj )
{
    return pObj->volume - sGetFillSize(pObj);
}

/** END of File *********************************************************/


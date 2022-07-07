#ifndef _LIB_PIPE_API_H_
#define _LIB_PIPE_API_H_


/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef enum
{
    LIB_PIPE_ERROR_NONE,
    LIB_PIPE_ERROR_EMPTY,
    LIB_PIPE_ERROR_FULL
} LibPipeError_t;

/************************************************************************/
/*  Function Prototype                                                  */
/************************************************************************/
Inst_t LibPipeCreate( const char* pNameStr, uint16_t volume , uint8_t objSize );
void   LibPipeDelete( Inst_t inst );

LibPipeError_t LibPipeWrite( Inst_t inst, void* pData );
LibPipeError_t LibPipeRead( Inst_t inst, void* pData );
LibPipeError_t LibPipeCheck( Inst_t inst, void* pData );

uint16_t LibPipeGetFillSize( Inst_t inst );
uint16_t LibPipeGetFreeSize( Inst_t inst );
uint16_t LibPipeGetTotalSize( Inst_t inst );

#endif /* LIB_PIPE_API_H */


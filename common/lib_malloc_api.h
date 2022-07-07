/*************************************************************************
* lib_malloc_api.h
*************************************************************************/
#ifndef LIB_MALLOC_API_H
#define LIB_MALLOC_API_H


/************************************************************************/
/*  Function Prototype                                                  */
/************************************************************************/
void*  LibMallocCreate( uint_t size );
void   LibMallocDelete( void* p );
uint_t LibMallocBytesAllocatedGet( void );
void   LibMallocLock( void );
void   LibMallocDeleteAll( void );

#endif 

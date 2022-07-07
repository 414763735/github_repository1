
#include "com_assert.h"

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define MEMPOOL_SIZE_IN_BYTES   (0x2000)
#define MEMPOOL_SIZE_IN_WORDS   ((MEMPOOL_SIZE_IN_BYTES>>2)+2)

/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/
static void sMemoryClear( uint_t size, uint_t ptr );

/************************************************************************/
/*  Static Variables                                                    */
/************************************************************************/
static uint32_t  sMemPool[MEMPOOL_SIZE_IN_WORDS]; /* Ensure memory pool location at 4-byte boundary */
static uint_t    sPtr     = 0;                    /* Pointer to next avaialble memory location      */
static bool      sbLock   = false;

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/
void* LibMallocCreate( uint_t size )
{
    uint_t words = ((size>>2)+1); /* Round up to nearest number of words (1 word = 4 bytes) */

    /* Check if memory pool is locked */
    APP_ASSERT(!sbLock);//

    if( MEMPOOL_SIZE_IN_WORDS > (sPtr+words) )
    {
        uint_t ptr = sPtr;
 
        /* Clear memory */
        sMemoryClear(words, sPtr);

        /* Increase pointer to next available memory location */
        sPtr += words;

        return (void*)(&sMemPool[ptr]);
    }
    else
    {
        APP_ASSERT(0);//
        return NULL;
    }
}

void LibMallocDelete( void* p )
{
    /* Make sure that delete pointer is in allocated memory space */
    APP_ASSERT((uint32_t*)p >= &sMemPool[0]);//
    APP_ASSERT((uint32_t*)p <  &sMemPool[sPtr]);//

    /* Make sure that delete pointer is on a even 4 byte boundary */
    APP_ASSERT(!((((uint8_t*)p) - ((uint8_t*)&sMemPool[0]))%4));//

    sPtr = (((uint8_t*)p) - ((uint8_t*)&sMemPool[0])) >> 2;
}

uint_t LibMallocBytesAllocatedGet( void )
{
    /* Return total amound of bytes allocated */
    return (uint_t)(sPtr<<2);
}

void LibMallocLock( void )
{
    sbLock = true;
}

void LibMallocDeleteAll( void )
{
    sbLock = false;
    sPtr   = 0;
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
static void sMemoryClear( uint_t size, uint_t ptr )
{
    uint32_t* pData = &sMemPool[ptr];

    while( size-- )
    {
        *pData = 0;
        pData++;
    }
}


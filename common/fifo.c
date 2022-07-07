#include "fifo.h"

#define MAX(x, y)             ( ((x) > (y)) ? (x) : (y) ) 
#define MIN(x, y)             ( ((x) < (y)) ? (x) : (y) ) 

/*
 *    Determine whether some value is a power of two, where zero is
 * *not* considered a power of two.
 */
#define is_power_of_2(n)    ((n) != 0 && (((n) & ((n) - 1)) == 0))

/**
 * FIFO_Init - allocates a new FIFO using a preallocated buffer
 * @buffer: the size of the internal buffer, this have to be a power of 2.
 * @size: the size of the internal buffer
 */
long int FIFO_Init(fifo_t *fifo, unsigned char *buffer, unsigned long int size)
{
    /* size must be a power of 2 */
    APP_ASSERT(is_power_of_2(size));//
    
    fifo->buffer = buffer;
    fifo->size = size;
    fifo->in = 0;
    fifo->out = 0;

    return 0;
}
#if 0  //to save memory

/**
 * FIFO_Put - puts some data into the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: the data to be added.
 * @len: the length of the data to be added.
 *
 * This function copies at most @len bytes from the @buffer into
 * the FIFO depending on the free space, and returns the number of
 * bytes copied.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned long int FIFO_Put(fifo_t *fifo, unsigned char *buffer, unsigned long int len)
{
    unsigned int l;

    len = MIN(len, fifo->size - fifo->in + fifo->out);

    /* first put the data starting from fifo->in to buffer end */
    l = MIN(len, fifo->size - (fifo->in & (fifo->size - 1)));
    memcpy(fifo->buffer + (fifo->in & (fifo->size - 1)), buffer, l);

    /* then put the rest (if any) at the beginning of the buffer */
    memcpy(fifo->buffer, buffer + l, len - l);

    fifo->in += len;

    return len;
}

/**
 * FIFO_Get - gets some data from the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @buffer: where the data must be copied.
 * @len: the size of the destination buffer.
 *
 * This function copies at most @len bytes from the FIFO into the
 * @buffer and returns the number of copied bytes.
 *
 * Note that with only one concurrent reader and one concurrent
 * writer, you don't need extra locking to use these functions.
 */
unsigned long int FIFO_Get(fifo_t *fifo, unsigned char *buffer, unsigned long int len)
{
    unsigned long int l;

    len = MIN(len, fifo->in - fifo->out);

    /* first get the data from fifo->out until the end of the buffer */
    l = MIN(len, fifo->size - (fifo->out & (fifo->size - 1)));
    memcpy(buffer, fifo->buffer + (fifo->out & (fifo->size - 1)), l);

    /* then get the rest (if any) from the beginning of the buffer */
    memcpy(buffer + l, fifo->buffer, len - l);

    fifo->out += len;

    return len;
}

#endif

/**
 * __fifo_putc - put a data into the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @c: the data to be added.
 */
long int FIFO_Putc(fifo_t *fifo, unsigned char c)
{
    /* if there is free space*/

    if((fifo->size - fifo->in + fifo->out) > 0)
    {    
        *(fifo->buffer + (fifo->in & (fifo->size - 1))) = c;
        fifo->in += 1;
        if (fifo->in >= fifo->size)
            fifo->in = 0;
        return 1;
    }

    return 0;
}

/**
 * FIFO_Getc - get adata from the FIFO, no locking version
 * @fifo: the fifo to be used.
 * @c: where the data must be copied.
 */
long int FIFO_Getc(fifo_t *fifo, unsigned char *c)
{
    if((fifo->in - fifo->out) > 0)
    {    
        *c = *(fifo->buffer + (fifo->out & (fifo->size - 1)));
        fifo->out += 1;
        if (fifo->out == fifo->size)
            fifo->out = 0;
        return 1;
    }

    return 0;
}

/**
 * FIFO_Length - returns the number of bytes available in the FIFO, no locking version
 * @fifo: the fifo to be used.
 */
unsigned long int FIFO_Length(fifo_t *fifo)
{
    return fifo->in - fifo->out;
}

/**
 * FIFO_Reset - removes the entire FIFO contents, no locking version
 * @fifo: the fifo to be emptied.
 */
void FIFO_Reset(fifo_t *fifo)
{
    fifo->in = fifo->out = 0;
}

/**
 */
void FIFO_Dropc(fifo_t *fifo, unsigned long int dorp_bytes)
{
	if ((fifo->in - fifo->out) >= (dorp_bytes & (fifo->size - 1)))
	{
		fifo->out += dorp_bytes & (fifo->size - 1);
	}
}



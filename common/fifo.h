#ifndef _FIFO_H_
#define _FIFO_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "com_assert.h"

typedef struct 
{
    unsigned char *buffer;       /* the buffer holding the data */
    unsigned long int size;      /* the size of the allocated buffer */
    unsigned long int in;        /* data is added at offset (in % size) */
    unsigned long int out;       /* data is extracted from off. (out % size) */
}fifo_t;

extern long int FIFO_Init(fifo_t *fifo, unsigned char *buffer, unsigned long int size);
extern long int FIFO_Putc(fifo_t *fifo, unsigned char c);
extern long int FIFO_Getc(fifo_t *fifo, unsigned char *c);
extern void     FIFO_Dropc(fifo_t *fifo, unsigned long int dorp_bytes);
//extern unsigned long int FIFO_Put(fifo_t *fifo, unsigned char *buffer, unsigned long int len);
//extern unsigned long int FIFO_Get(fifo_t *fifo, unsigned char *buffer, unsigned long int len);
extern unsigned long int FIFO_Length(fifo_t *fifo);
extern void FIFO_Reset(fifo_t *fifo);

#endif


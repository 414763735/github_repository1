#ifndef _EQ_MSG_H_
#define _EQ_MSG_H_

typedef struct
{
  uint8_t type;
	uint8_t id;
	uint8_t *data;
}EqMsg_t;


typedef struct
{
	EqMsg_t *queue;		/* the buffer holding the data */
	unsigned long size;			/* the size of the allocated buffer */
	unsigned long in;			/* data is added at offset (in % size) */
	unsigned long out;			/* data is extracted from off. (out % size) */
} eqMsgQ_t;

unsigned int eqMsgQueueInit (eqMsgQ_t * msgQueue, EqMsg_t *buffer, unsigned long size);
int eqMsgGet (eqMsgQ_t * msgQueue, EqMsg_t *msg);
int eqMsgPut (eqMsgQ_t * msgQueue, EqMsg_t *msg);
void eqMsgReset(eqMsgQ_t * msgQueue);

#endif

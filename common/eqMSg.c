#include "com_assert.h"
#include "com_string.h"
#include "eqMsg.h"

unsigned int eqMsgQueueInit (eqMsgQ_t * msgQueue, EqMsg_t *queue, unsigned long size)
{

	msgQueue->queue = queue;
	msgQueue->size = size;
	msgQueue->in = 0;
	msgQueue->out = 0;

	return 0;
}

int eqMsgGet (eqMsgQ_t * msgQueue, EqMsg_t *msg)
{
	if ((msgQueue->in - msgQueue->out) > 0)
	{
		*msg = *(msgQueue->queue + (msgQueue->out & (msgQueue->size - 1)));
		msgQueue->out += 1;
		return 1;
	}

	return 0;
}

int eqMsgPut (eqMsgQ_t * msgQueue, EqMsg_t *msg)
{
	/* if there is free space */
	if ((msgQueue->size - msgQueue->in + msgQueue->out) > 0)
	{
		*(msgQueue->queue + (msgQueue->in & (msgQueue->size - 1))) = *msg;
		msgQueue->in += 1;
		return 1;
	}

	return 0;
}

void eqMsgReset(eqMsgQ_t * msgQueue)
{
	msgQueue->in = msgQueue->out = 0;
}


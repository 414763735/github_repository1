
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "com_assert.h"

#include "hal_system_timer.h"

#include "lib_obj_api.h"
#include "lib_malloc_api.h"

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define LibDebugPrintf   DBG_MSG

/************************************************************************/
/*  Type Definitions                                                    */
/************************************************************************/
typedef enum
{
	STATE_STOPPED,
	STATE_STARTED
} State_t;

typedef struct Hdr_s
{
	LibObjList_t 	*pInstList;		//指向实例List的指针的指针。 比如 指向LibObjList_si_hal_gpio的指针。
	const char 		*pInstStr;		//一个字符串指针，指向如 "gpio"的指针。
	struct Hdr_s 	*pNxtHdr;		//指向下一个Header。这是一个循环链表，一个元时(即只被创建了一次的时候)，自己指向自己。
	uint32_t 		 logFlags;
	State_t 		 state;
} Hdr_t;

typedef struct Lst_s
{
	const char 		*pClassStr;		//实例的名字 - 一个字符串指针。 指向如 "si_hal_gpio"的指针
	Hdr_t 			*pLstHdr;		//该实例的Header的头指针。因为Header是一个循环链表，所以可能指向的是链表中的某一个。
	struct Lst_s 	*pNxtLst;		//下一个实例的List的头指针。
} Lst_t;

/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/
static void sAddList (Lst_t * pLstNew);
static void sRemoveList (Lst_t * pLstDel);
static uint32_t sLogType2BitMask (LibObjLogType_t logType);
static void sRemoveCharacters (const char *pStr, char chr);
static uint_t sLogLimited (uint_t size, const char *pStr);
static Hdr_t *sPrevHeaderGet (Hdr_t * pHdr);
//static void   sPreInsert( Hdr_t* pDesHdr, Hdr_t* pSrcHdr );
static void sPostInsert (Hdr_t * pDesHdr, Hdr_t * pSrcHdr);
static void sInsertHeader (Hdr_t * pHdr);
static void sRemoveHeader (Hdr_t * pHdr);

/************************************************************************/
/*  Static Variables                                                    */
/************************************************************************/
static Lst_t *spLstFirst = NULL;

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/

//*****************************************************************************
//  LibObjCreate
//! 创建一个常规实例(LibObj)，分配内存，实例包含三部分，List,Header,Entity(实体)。
//! 并将List头指针加入 spLstFirst 的List列表中，将Header头指针加入Header的循环链表中。
//! 当该实例的LIST是被创建过的，如SeqTimerObj，则不再创建List，只是将Header加入Header的循环列表。
//!
//! \param * pInstList  指向实例List的指针的指针。
//! \param * pClassStr  实例的名字 - 一个字符串指针。 指向如 "si_hal_gpio"， 将放在 List中。
//! \param * pInstStr   实例的实体名字 - 一个字符串指针，指向如 "gpio"， 将放在 Header中。
//! \param   size       实例的实体的大小。
//! 
//! \return  若创建成功，返回实例的实体的头指针，否则是NULL。
//
//*****************************************************************************
void *LibObjCreate (LibObjList_t * pInstList, const char *pClassStr, const char *pInstStr, LibObjSize_t size)
{
	Hdr_t *pHdr;
	Lst_t *pLst;

	if (*pInstList)
	{
		pLst = (Lst_t *) * pInstList;

		/* Check for correct list */
		APP_ASSERT (pClassStr == pLst->pClassStr);//
	}
	else
	{
		/* Create new list */
		pLst = (Lst_t *) LibMallocCreate (sizeof (Lst_t));
		APP_ASSERT (pLst);//
		pLst->pClassStr = pClassStr;
		pLst->pLstHdr = NULL;
		pLst->pNxtLst = NULL;
		*pInstList = (LibObjList_t) pLst;

		/* Create link between each list of instantiations */
		sAddList (pLst);
	}

	/* Request memory for instance header */
	pHdr = (Hdr_t *) LibMallocCreate (sizeof (Hdr_t) + size);
	if (pHdr)
	{
		/* Configure header */
		pHdr->pInstList = pInstList;
		pHdr->pInstStr = pInstStr;
		pHdr->pNxtHdr = NULL;
		pHdr->state = STATE_STOPPED;

		/* Insert instance to linked list */
		sInsertHeader (pHdr);
		APP_ASSERT (pHdr->pNxtHdr);//

		return ((char *) pHdr) + sizeof (Hdr_t);
	}
	return NULL;
}

void LibObjDelete (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));
	Lst_t *pLst = (Lst_t *) * pHdr->pInstList;

	/* Remove instance from linked list */
	APP_ASSERT (pHdr->pNxtHdr);//
	sRemoveHeader (pHdr);
	APP_ASSERT (!pHdr->pNxtHdr);//

	/* Clear List reference in parent (user) */
	if (!pLst->pLstHdr)
	{
		*pHdr->pInstList = NULL;
	}

	/* Delete instance memory */
	LibMallocDelete (pHdr);
	if (!pLst->pLstHdr)
	{
		/* Remove list of instances */
		sRemoveList (pLst);

		/* Delete list memory */
		LibMallocDelete (pLst);
	}
}

void LibObjStart (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));

	pHdr->state = STATE_STARTED;
}

void LibObjStop (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));

	pHdr->state = STATE_STOPPED;
}

bool LibObjStartedIs (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));

	return (STATE_STARTED == pHdr->state) ? (true) : (false);
}

const char *LibObjNameModuleGet (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));
	Lst_t *pLst = (Lst_t *) * pHdr->pInstList;

	return pLst->pClassStr;
}

const char *LibObjNameInstanceGet (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));

	return pHdr->pInstStr;
}

bool LibObjCheck (LibObjList_t * pInstList, void *p)
{
	if (!p)
	{
		return false;
	}
	else
	{
		Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));

		/* Check if object is registered to the same list */
		return (pInstList == pHdr->pInstList);
	}
}

void *LibObjFirstGet (LibObjList_t instList)
{
	if (instList)
	{
		Lst_t *pLst = (Lst_t *) instList;

		return ((char *) pLst->pLstHdr->pNxtHdr) + sizeof (Hdr_t);
	}
	return NULL;
}

void *LibObjLastObjectGet (LibObjList_t instList)
{
	if (instList)
	{
		Lst_t *pLst = (Lst_t *) instList;

		return ((char *) pLst->pLstHdr) + sizeof (Hdr_t);
	}
	return NULL;
}

void *LibObjNextGet (void *p)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));
	Lst_t *pLst = (Lst_t *) * pHdr->pInstList;

	APP_ASSERT (pHdr);//
	APP_ASSERT (pLst);//
	if (pHdr != pLst->pLstHdr)
		return ((char *) pHdr->pNxtHdr) + sizeof (Hdr_t);

	/* Reached end of list */
	return NULL;
}

void LibObjMove (void *pDes, void *pSrc)
{
	Hdr_t *pSrcHdr = (Hdr_t *) ((char *) pSrc - sizeof (Hdr_t));
	Lst_t *pLst = (Lst_t *) * pSrcHdr->pInstList;

	/* No mvove is needed if pSrc and pDes pointing to the same object */
	if (pDes == pSrc)
		return;

	/* Make sure that source is a valid pointer */
	APP_ASSERT (pSrc);//

	/* Make sure that source object is currently linked in */
	APP_ASSERT (pSrcHdr->pNxtHdr);//

	/* Remove source object from linked list */
	sRemoveHeader (pSrcHdr);

	/* With a valid destination object provided the source object will be replaced to directly behind the destination object. */
	/* However in case no destination is provided the source object will be replace to the first position.                    */
	if (pDes)
	{
		Hdr_t *pDesHdr = (Hdr_t *) ((char *) pDes - sizeof (Hdr_t));

		/* Make sure that both instantiations belong to the same list (type) */
		APP_ASSERT (pDesHdr->pInstList == pSrcHdr->pInstList);//

		/* Make sure that destination object is a linked in object */
		APP_ASSERT (pDesHdr->pNxtHdr);//

		/* Insert in linked list */
		sPostInsert (pDesHdr, pSrcHdr);

		/* Update last poiter if inserted at end of list */
		if (pDesHdr == pLst->pLstHdr)
		{
			pLst->pLstHdr = pSrcHdr;
		}
	}
	else
	{
		if (pLst->pLstHdr)
		{
			/* Insert as first object in list */
			sPostInsert (pLst->pLstHdr, pSrcHdr);
		}
		else
		{
			pSrcHdr->pNxtHdr = pSrcHdr;	/* Ensure circular list */
			pLst->pLstHdr = pSrcHdr;
		}
	}
}

void *LibObjPointerGetByName (LibObjList_t instList, const char *pClassStr)
{
	if (instList)
	{
		Lst_t *pLst = (Lst_t *) instList;

		if (pLst->pLstHdr)
		{
			Hdr_t *pFirstHdr = pLst->pLstHdr->pNxtHdr;
			Hdr_t *pHdr = pFirstHdr;

			do
			{
				if (!strcmp (pHdr->pInstStr, pClassStr))
				{
					return ((char *) pHdr) + sizeof (Hdr_t);
				}
				pHdr = pHdr->pNxtHdr;
			}
			while (pFirstHdr != pHdr);
		}
	}
	return NULL;
}

void LibObjLog (void *p, LibObjLogType_t logType, const char *pFrm, ...)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));
	uint32_t bitMsk = sLogType2BitMask (logType);

	if (pHdr->logFlags & bitMsk)
	{
		uint32_t i, min, sec, ms;
		Lst_t  *pLst = (Lst_t *) * pHdr->pInstList;
		int32_t mSec = HAL_GetCurrentMsCount();
		uint_t  tot = 0;

		/* Print time stamp */
		i = mSec % 3600000;
		ms = i % 1000;
		sec = (i / 1000) % 60;
		min = (i / 1000) / 60;

		LibDebugPrintf ("[%02d:%02d:%03d]", min, sec, ms);

		/* Print module name */
		tot += sLogLimited (12, pLst->pClassStr);

		/* Print instance name */
		if (pHdr->pInstStr && strlen (pHdr->pInstStr))
		{
			/* Separation character */
			LibDebugPrintf (".");
			tot++;

			tot += sLogLimited (12, pHdr->pInstStr);
		}

		/* Print alignment space characters */
		tot = (24 < tot) ? (0) : (24 - tot);
		while (tot--)
			LibDebugPrintf (" ");

		/* Print end of preamble */
		LibDebugPrintf (": ");
		/* print notification string */
		LibDebugPrintf (pFrm);
		LibDebugPrintf ("\r\n");
		
	}
}

void LibObjLogTypeSet (void *p, LibObjLogType_t logType, bool isTrue)
{
	Hdr_t *pHdr = (Hdr_t *) ((char *) p - sizeof (Hdr_t));
	uint32_t bitMsk = sLogType2BitMask (logType);

	pHdr->logFlags = (isTrue) ? (pHdr->logFlags | bitMsk) : (pHdr->logFlags & (~bitMsk));
}

/************************************************************************/
/*  Local Functions                                                     */
/************************************************************************/
static void sAddList (Lst_t * pLstNew)
{
	if (spLstFirst)
	{
		Lst_t *pLst = spLstFirst;

		while (pLst->pNxtLst)
		{
			pLst = pLst->pNxtLst;
		}
		pLst->pNxtLst = pLstNew;
	}
	else
	{
		spLstFirst = pLstNew;
	}
}

static void sRemoveList (Lst_t * pLstDel)
{
	APP_ASSERT (spLstFirst);//
	if (spLstFirst == pLstDel)
	{
		spLstFirst = spLstFirst->pNxtLst;
	}
	else
	{
		Lst_t *pLst = spLstFirst;

		while (pLst->pNxtLst)
		{
			if (pLst->pNxtLst == pLstDel)
				break;

			pLst = pLst->pNxtLst;
		}
		APP_ASSERT (pLst->pNxtLst);//
		pLst->pNxtLst = pLstDel->pNxtLst;
	}
}

static uint32_t sLogType2BitMask (LibObjLogType_t logType)
{
	switch (logType)
	{
	case LIB_OBJ_LOG_TYPE__INFO:
		return 1 << 0;
	case LIB_OBJ_LOG_TYPE__WARNING:
		return 1 << 1;
	case LIB_OBJ_LOG_TYPE__ERROR:
		return 1 << 2;
	case LIB_OBJ_LOG_TYPE__DEBUG:
		return 1 << 3;

	default:
		APP_ASSERT (0);//
		break;
	}
	return 0;
}

static void sRemoveCharacters (const char *pStr, char chr)
{
	const char *p1 = pStr;
	char *p2 = (char *) pStr;

	while (*p1)
	{
		if (*p1 != chr)
		{
			/* Copy/move character */
			*p2 = *p1;

			p2++;
		}
		p1++;
	}
	*p2 = 0;
}

static uint_t sLogLimited (uint_t size, const char *pStr)
{
	uint_t i;

	for (i = 0; i < size; i++)
	{
		/* Check for end of string */
		if (!(*pStr))
			break;

		LibDebugPrintf ("%c", *pStr);
		pStr++;
	}
	return i;
}

static Hdr_t *sPrevHeaderGet (Hdr_t * pHdr)
{
	Hdr_t *pTempHdr = pHdr;

	while (pHdr != pTempHdr->pNxtHdr)
		pTempHdr = pTempHdr->pNxtHdr;

	return pTempHdr;
}

#if 0							// Currently not used
static void sPreInsert (Hdr_t * pDesHdr, Hdr_t * pSrcHdr)
{
	Hdr_t *pPrvHdr = sPrevHeaderGet (pDesHdr);

	pSrcHdr->pNxtHdr = pDesHdr;
	pPrvHdr->pNxtHdr = pSrcHdr;
}
#endif

static void sPostInsert (Hdr_t * pDesHdr, Hdr_t * pSrcHdr)
{
	pSrcHdr->pNxtHdr = pDesHdr->pNxtHdr;
	pDesHdr->pNxtHdr = pSrcHdr;
}

static void sInsertHeader (Hdr_t * pHdr)
{
	Lst_t *pLst = (Lst_t *) * pHdr->pInstList;

	APP_ASSERT (!pHdr->pNxtHdr);//
	if (pLst->pLstHdr)
	{
		sPostInsert (pLst->pLstHdr, pHdr);
	}
	else
	{
		pHdr->pNxtHdr = pHdr;	/* Ensure circular list */
	}
	pLst->pLstHdr = pHdr;
}

static void sRemoveHeader (Hdr_t * pHdr)
{
	Lst_t *pLst = (Lst_t *) * pHdr->pInstList;

	APP_ASSERT (pHdr->pNxtHdr);//
	/* check if last instance removal */
	if (pHdr == pHdr->pNxtHdr)
	{
		/* Clear reference to linked list */
		pLst->pLstHdr = NULL;
	}
	else
	{
		Hdr_t *pPrvHdr = sPrevHeaderGet (pHdr);

		/* remove instance out of linked list */
		pPrvHdr->pNxtHdr = pHdr->pNxtHdr;

		/* Change last instance reference in case last instance is removed */
		if (pLst->pLstHdr == pHdr)
			pLst->pLstHdr = pPrvHdr;
	}

	/* Clear next reference to indicate that instance has removed from list */
	pHdr->pNxtHdr = NULL;
}

/** END of File *********************************************************/

/***********************************************************************
* Copyright (c) 2016, Hansong (Nanjing) Technology Ltd.
* All rights reserved.
* 
* ----File Info ----------------------------------------------- -----------------------------------------
* Name:     lib_console.c
* Author:   Baron.Peng
* Date:     2016-2-23
* Summary:  ¿ØÖÆÌ¨Èí¼þ¡£
*
* ----Latest Version----------------------------------------------- ---------------------------------
* Version: vc1.00
* Date:    2016-2-23
* Author:  Baron.peng
* Description: First create,
*
* ----Revision History----------------------------------------------- ------------------------------
*
**********************************************************************/
//#include "pic32mx_headers.h"
#include "com_assert.h"
#include "fifo.h"

#include "lib_semaphore.h"

#include "com_string.h"
#include "hal_system_uart.h"
#include "hal_system_timer.h"
#include "lib_console.h"

#if _UART_DEBUG_PRINT_
#define TCMD_DBG                DBG_MSG
#define TCMD_MSG                DBG_TIME("RS232: ");DBG_MSG
#define TRS232_MSG              DBG_TIME("RS232: ");DBG_MSG
#else
#define TCMD_DBG(...)
#define TCMD_MSG(...)
#define TRS232_MSG(...)
#endif

#if _CONSOLE_FUNC_EN_

LIB_OBJ_MODULE_DEF (lib_cons);

/************************************************************************/
/*  Constants Definitions                                               */
/************************************************************************/
#define ConsolePollingRateMs     300

/************************************************************************/
/*  Type define Definitions                                             */
/************************************************************************/
typedef struct
{
    const char *nameStr;
    const char *descStr;
    uint32_t    nameStrLen;
    LibConsoleCbFunc_t cbFunc;
} Obj_t;

static Inst_t consInst;

static char uartRxBuffer[MSG_MAX_LEN];
static unsigned char uartRxIndex;


/************************************************************************/
/*  Local Prototypes                                                    */
/************************************************************************/

static void sConsole_UpdateServ (void);
static void sConsole_PollHandlerTask (Inst_t inst);
static void sConsole_UpdateCmdListSequ (Obj_t * pObj);

static unsigned char sConsole_HelpCmdPrint (Inst_t parms);

/************************************************************************/
/*  Application Interface                                               */
/************************************************************************/

void AppConsole_CreateServ (void)
{
    /* Initialize instance */
    consInst = LibSeqTimerCreate ("console", sConsole_PollHandlerTask, NULL, LIB_SEQ_PRIOR__CONSOLE_POLLING);
    APP_ASSERT (consInst);//

    LibSeqTimerStart (consInst, ConsolePollingRateMs, ConsolePollingRateMs);

    HAL_Uart5ConsoleCancel();
    HAL_Uart5ConsoleRegist (sConsole_UpdateServ,115200);

    AppConsole_RegisterCommand ("?",     "Print this message", sConsole_HelpCmdPrint);
    AppConsole_RegisterCommand ("help",  "Print this message", sConsole_HelpCmdPrint);
}

bool AppConsole_RegisterCommand (const char *pNameStr, const char *discripStr, LibConsoleCbFunc_t cbFunc)
{
    Obj_t *pObj = NULL;

    APP_ASSERT (pNameStr);//
    APP_ASSERT (cbFunc);//

    /* Allocate memory for object */
    pObj = (Obj_t *) LIB_OBJ_CREATE (pNameStr, sizeof (Obj_t));
    APP_ASSERT (pObj);//

    pObj->nameStr = pNameStr;
    pObj->descStr = discripStr;
    pObj->cbFunc = cbFunc;
    pObj->nameStrLen = cus_strlen (pObj->nameStr);

    sConsole_UpdateCmdListSequ (pObj);
    return true;
}


/*============================================================================
*
*
*===========================================================================*/

static char *sRemoveSpaces (char *Command)
{
    while (*Command == ' ')
    {
        Command++;
    }
    return (Command);
}

static void sSwitch2littleCase(char *Command)
{
    char *str = Command;
    while (*str != '\0')
    {
        if ((*str >= 'A')&&(*str <= 'Z'))
        {
            *str = 'a' + (*str - 'A');
        }
        str++;
    }
}

static bool sConsole_MsgHandler (char *buffer)
{
    Obj_t *pObj = (Obj_t *) LIB_OBJ_FIRST ();
    char  *buf;
    char  *paraStr;
    bool   ret;

    ret = false;
    //sSwitch2littleCase(buffer);
    buf = sRemoveSpaces(buffer);

    if (*buf == 0)
        return true;
    
    while (pObj)
    {
        if (!cstrncmp ((void *) buf, (void *) (pObj->nameStr), pObj->nameStrLen))
        {
            if ((buf[pObj->nameStrLen] == 0) || (buf[pObj->nameStrLen] == ' '))
            {
                paraStr = sRemoveSpaces (buf + pObj->nameStrLen);
                if (!pObj->cbFunc (paraStr))
                {
                    TCMD_DBG ("\r\nCommand execution failed\r\n");
                }
                else
                {
                    ret = true;
                }
                break;
            }
        }

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT (pObj);
    }

    if (!pObj)
    {
        TCMD_DBG ("\r\nUnknown command\r\n");
    }
    TCMD_DBG ("\r\n");

    return ret;
}

static unsigned char sConsole_HelpCmdPrint (Inst_t parms)
{
    Obj_t *pObj = (Obj_t *) LIB_OBJ_FIRST ();
    char Spaces[22];

    Spaces[20] = 0;

    TCMD_DBG ("\r\nCommand   Description and parameters (All data in hex)\r\n\r\n");

    while (pObj)
    {
        cmemset ((void *) Spaces, 0x20, 20);
        cmemcpy ((void *) Spaces, (void *) (pObj->nameStr), cus_strlen (pObj->nameStr));
        TCMD_DBG ((const char *) Spaces);
        TCMD_DBG (pObj->descStr);
        TCMD_DBG (".\r\n");

        /* Go to next timer instance */
        pObj = LIB_OBJ_NEXT (pObj);
    }
    return 1;
}

static void sConsole_UpdateCmdListSequ (Obj_t * pObj)
{
    Obj_t *pTmpObj = (Obj_t *) LIB_OBJ_FIRST ();

    if (*(pTmpObj->nameStr) > *(pObj->nameStr))
    {
        /* Move object to first place in list */
        LIB_OBJ_MOVE (NULL, pObj);
    }
    else
    {
        Obj_t *pPrvObj;
        do
        {
            pPrvObj = pTmpObj;
            /* Go to next instance */
            pTmpObj = LIB_OBJ_NEXT (pTmpObj);
        }
        while ((pTmpObj) && (*(pTmpObj->nameStr) <= *(pObj->nameStr)));

        /* Move object to directly after pPrvObj */
        LIB_OBJ_MOVE (pPrvObj, pObj);
    }
}

static void sConsole_UpdateServ (void)
{
    LibSeqTimerStart (consInst, 0, ConsolePollingRateMs);
}

static bool sRxBufferNotEmpty(void)
{
    return uartRxIndex;
}

static bool sRxBufferPush(char data)
{
    if (uartRxIndex < MSG_MAX_LEN)
    {
        uartRxBuffer[uartRxIndex] = data;
        uartRxIndex++;
    }
    else
    {
        uartRxIndex = 0;
    }
    return uartRxIndex;
}

static bool sRxBufferBackspace(void)
{
    if (uartRxIndex > 0)
    {
        uartRxIndex--;
    }
    return uartRxIndex;
}

static void sRxBufferSave(void)
{
    uartRxBuffer[uartRxIndex] = 0;
    uartRxIndex = 0;
}

static void sConsole_PollHandlerTask (Inst_t inst)
{
    char RecByte;
    bool ByteAvail;

    ByteAvail = RS232_GetChar (&RecByte);

    while (ByteAvail)
    {
        switch (RecByte)
        {
            case 0x0D:             /* CR */
            case 0x0A:             /* LF */
            {
                if (sRxBufferNotEmpty())
                {
                    sRxBufferSave();
                    if (sConsole_MsgHandler ((char *) uartRxBuffer))
                    {
                        ;
                    }
                }
                else if (RecByte == 0x0D)
                {
                    TRS232_MSG (" \r\n");
                }
                break;
            }
            case 8:
            {
                sRxBufferBackspace();
                RS232_PutChar (RecByte);
                break;
            }
            default:
            {
                RS232_PutChar (RecByte);    //
                if (!sRxBufferPush(RecByte))
                {
                    TCMD_DBG ("cmd overflow!\r\n");
                }
                break;
            }
        }
        ByteAvail = RS232_GetChar (&RecByte);
    }

}

#endif

void RS232_PutChar (unsigned char dat)
{
    HAL_Uart5PutByte (dat);
}

bool RS232_GetChar (unsigned char *dat)
{
    return HAL_Uart5GetByte (dat);
}

void RS232_ClearBuffer (void)
{
    uint8_t RecByte;
    do
    {
    }
    while (HAL_Uart5GetByte (&RecByte));
}

unsigned char RS232_ReadBuffer (unsigned char *dat)
{
    unsigned char RecByte, ByteAvail;
    ByteAvail = (unsigned char) HAL_Uart5GetByte (&RecByte);
    *dat = RecByte;
    return ByteAvail;
}


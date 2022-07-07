#ifndef _CONSOLE_H_
#define _CONSOLE_H_

#include "com_string.h"

#define _CONSOLE_FUNC_EN_      1

typedef struct
{
    char *CmdName;
    char *CmdHelp;
    unsigned char  (*CmdHandler)(char  *Cmd, char  *Parms);
}UartCmd_t;

typedef unsigned char (*LibConsoleCbFunc_t)( Inst_t parms );

extern void    RS232_PortEnable(void);
extern void    RS232_PutChar(uint8_t dat);
extern bool    RS232_GetChar(uint8_t *dat);
extern void    RS232_ClearBuffer(void);
extern uint8_t RS232_ReadBuffer(uint8_t *dat);

extern void    AppConsole_CreateServ(void);
extern bool    AppConsole_RegisterCommand(const char* pNameStr, const char* discripStr, LibConsoleCbFunc_t cbFunc);

#endif


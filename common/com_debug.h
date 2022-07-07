#ifndef _DEBUG_CONSOLE_H_
#define _DEBUG_CONSOLE_H_

#include "GenericTypeDefs.h"

#define _UART_DEBUG_PRINT_     1
#define _ASSERT_PRINT_         0

#define _INTERNAL_PROGRAM_     1

#define HW_VC1                 0
#define HW_VC2                 1
#define HW_VC3                 2

#define HardWareVer            HW_VC2

#define _RF_TEST_              0

#define _GC_TEST_              1
#define _CANTON_LINK_          0 // set to 1 only when release it for factory production!!!

#define _D83_BAND_SWITCH_      1

#define SWITCH_HDMI_P0P2       1

#define DOLBY_DTS_CERT         0

#define MSG_MAX_LEN            128

/*
#define NONE         "\033[m" 
#define RED          "\033[0;32;31m" 
#define LIGHT_RED    "\033[1;31m" 
#define GREEN        "\033[0;32;32m" 
#define LIGHT_GREEN  "\033[1;32m" 
#define BLUE         "\033[0;32;34m" 
#define LIGHT_BLUE   "\033[1;34m" 
#define DARY_GRAY    "\033[1;30m" 
#define CYAN         "\033[0;36m" 
#define LIGHT_CYAN   "\033[1;36m" 
#define PURPLE       "\033[0;35m" 
#define LIGHT_PURPLE "\033[1;35m" 
#define BROWN        "\033[0;33m" 
#define YELLOW       "\033[1;33m" 
#define LIGHT_GRAY   "\033[0;37m" 
#define WHITE        "\033[1;37m"

\033[0m     关闭所有属性   
\033[1m     设置高亮度   
\033[4m     下划线   
\033[5m     闪烁   
\033[7m     反显   
\033[8m     消隐   
\033[30m   --   \033[37m   设置前景色   
\033[40m   --   \033[47m   设置背景色   
\033[nA     光标上移n行   
\033[nB     光标下移n行   
\033[nC     光标右移n行   
\033[nD     光标左移n行   
\033[y;xH   设置光标位置   
\033[2J     清屏   
\033[K      清除从光标到行尾的内容   
\033[s      保存光标位置   
\033[u      恢复光标位置   
\033[?25l   隐藏光标   
\033[?25h   显示光标
*/

extern signed int cprintf(const char *pFormat, ...);
extern signed int cprintf_time(const char *Postfix);
extern signed int cprintf_freq(const char *Postfix);
extern void       cPrintChar(signed int c);

#if _UART_DEBUG_PRINT_
  #define DBG_MSG              cprintf
  #define DBG_TIME             cprintf_time
  #define DBG_COLOR(s,...)     DBG_MSG ("\033[0;32;32m "); DBG_MSG(s,__VA_ARGS__);DBG_MSG ("\033[0m\r\n")
  #define DBG_FREQ             cprintf_freq
  #define DBG_CHAR             cPrintChar
#else
  #define DBG_MSG_F  cprintf

  #define DBG_MSG(...)
  #define DBG_TIME(...)
  #define DBG_COLOR(...)
  #define DBG_FREQ(...)
  #define DBG_CHAR(c)
#endif

#endif



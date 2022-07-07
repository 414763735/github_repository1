/***********************************************************************
* Copyright (c) 2016, Hansong (Nanjing) Technology Ltd.
* All rights reserved.
* 
* ----File Info ----------------------------------------------- -----------------------------------------
* Name:     lib_xmode.c
* Author:   Baron.Peng
* Date:     2016-2-23
* Summary:  XMODE 传输协议层
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

#include "hal_platform.h"
#include "com_assert.h"
#include "lib_console.h"
#include "xmodem.h"

#include "hal_system_gpio.h"
#include "hal_system_timer.h"
#include "app_fw_update.h"
#include "dev_ls9.h"
#include "Dev_tubedisplay.h"

#if 1
#define  xmode_printf         DBG_MSG
#else
#define  xmode_printf(...) 
#endif

#if _INTERNAL_PROGRAM_

#define SOH  0x01
#define STX  0x02
#define EOT  0x04
#define ACK  0x06
#define NAK  0x15
#define CAN  0x18
#define CTRLZ 0x1A

//SOH 1 01H 数据块开始
//EOT 4 04H 发送结束
//ACK 6 06H 认可响应
//NAK 21 15H 不认可响应 对于CRC校验的协议软件，本信号用字母“C”（43H）代替。
//DLE 16 10H 中止数据连接
//X-on 17 11H 数据传送启动 当通信双方的速度不一致时，可采用该字符来调节通信速度，比如接收方速度太慢而导致接收缓冲器满时，发送“X-off”给发送方，使发送方暂停发送数据。相当于RS232接口的DSR,CTS等信号。
//X-off 19 13H 数据传送停止
//SYN 22 16H 同步
//CAN 24 18H 撤销传送


#define DLY_1S 150000
#define MAXRETRANS 25

xmodeCbFunc xCbFunc;

/* CRC16 implementation acording to CCITT standards */

unsigned char xbuff[1030]; /* 1024 for XModem 1k + 3 head chars + 2 crc + nul */

static const unsigned short crc16tab[256]= {
    0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
    0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
    0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
    0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
    0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
    0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
    0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
    0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
    0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
    0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
    0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
    0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
    0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
    0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
    0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
    0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
    0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
    0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
    0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
    0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
    0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
    0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
    0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
    0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
    0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
    0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
    0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
    0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
    0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
    0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
    0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
    0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
};

static unsigned char xm_getchar(unsigned char *dat)
{
    return (RS232_ReadBuffer(dat));
}

static unsigned char xm_putchar(unsigned char dat)
{
    RS232_PutChar(dat);
    return 0;
}
  
unsigned char _inbyte(unsigned int timeout) 
{
    unsigned char ch;
    while (timeout--) {
        if (xm_getchar(&ch))
            return ch;
    }
    return 0;
}

unsigned short crc16_ccitt(const void *buf, int len)
{
    int counter;
    unsigned short crc = 0;
    for( counter = 0; counter < len; counter++)
        crc = (crc<<8) ^ crc16tab[((crc>>8) ^ *(char *)buf++)&0x00FF];
    return crc;
}

static int check(int crc, const unsigned char *buf, int sz)
{
    if (crc) {
        unsigned short crc = crc16_ccitt(buf, sz);
        unsigned short tcrc = (buf[sz]<<8)+buf[sz+1];
        if (crc == tcrc)
            return 1;
    }
    else {
        int i;
        unsigned char cks = 0;
        for (i = 0; i < sz; ++i) {
            cks += buf[i];
        }
        if (cks == buf[sz])
        return 1;
    }

    return 0;
}

static void flushinput(void)
{
    while (_inbyte(DLY_1S) > 0)
        continue;
}

long xmodemReceive(long destsz)
{
    unsigned char *p;
    int bufsz, crc = 0;
    unsigned char trychar = 'C';
    unsigned char packetno = 1;
    long len = 0;
    int i, c;
    
    int retry, retrans = MAXRETRANS;

    for(;;) {
        for( retry = 0; retry < 240; ++retry) {  // approx 100 seconds allowed to make connection
            if (trychar) 
                xm_putchar(trychar);
            if ((c = _inbyte(DLY_1S))) {
                switch (c) {
                case SOH:
                    bufsz = 128;
                    goto start_recv;
                case STX:
                    bufsz = 1024;
                    goto start_recv;
                case EOT:
                    flushinput();
                    xm_putchar(ACK);
                    return len; /* normal end */
                case CAN:
                    if ((c = _inbyte(DLY_1S)) == CAN) {
                        flushinput();
                        xm_putchar(ACK);
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
        }
        if (trychar == 'C') { 
            trychar = NAK; 
            continue;
        }
        flushinput();
        xm_putchar(CAN);
        xm_putchar(CAN);
        xm_putchar(CAN);
        return -2; /* sync error */

    start_recv:
        if (trychar == 'C') 
            crc = 1;
        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
            if ((c = _inbyte(DLY_1S)) < 0) 
                goto reject;
            *p++ = c;
        }

        if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
                check(crc, &xbuff[3], bufsz)) {
            if (xbuff[1] == packetno)    
            {
                long count = destsz - len;
                if (count > bufsz) 
                    count = bufsz;
                if (count > 0) {
                    //my_memcpy (&dest[len], &xbuff[3], count);
                    len += count;
                    //DSP__BufferProgramming(&xbuff[3], count);
                    xCbFunc(&xbuff[3], count);
                }
                ++packetno;
                retrans = MAXRETRANS+1;
            }
            if (--retrans <= 0) {
                flushinput();
                xm_putchar(CAN);
                xm_putchar(CAN);
                xm_putchar(CAN);
                return -3; /* too many retry error */
            }
            xm_putchar(ACK);
            continue;
        }
    reject:
        flushinput();
        xm_putchar(NAK);
    }
}

int main_xmodem(xmodeCbFunc cbFunc)
{
    long st;

    APP_ASSERT(cbFunc);//

    xCbFunc = cbFunc;

    xmode_printf ("\r\nPlease send upgrade file using the xmodem protocol from your terminal emulator now...\r\n");

    /* the following should be changed for your environment:
       0x30000 is the download address,
       65536 is the maximum size to be written at this address
     */
     
    RS232_ClearBuffer();
    st = xmodemReceive(0x200000);//2048x1024
    if (st < 0) {
        xmode_printf ("\r\nXmodem receive error: status: %d\r\n", st);
    }
    else  {
        xmode_printf ("\r\nXmodem successfully received %d bytes\r\n", st);
    }
    RS232_ClearBuffer();

    return 0;
}

unsigned char _checkByte(uint32_t timeout) 
{
    unsigned char ch;
    uint32_t startTime;

    ch = 0;
    startTime = HAL_GetCurrentMsCount();
    while (HAL_GetElapsedMs(startTime) < timeout) {
        if (LIBM_UART_GET(&ch)){
            return ch;
        }
    }
    xmode_printf ("xmoden get nothing\r\n");
    return 0;
}


static void _xflushIn(void)
{
    while (_checkByte(1000) > 0)
        continue;
}

const  char strUpdating[] = {"UPDATING   "};
static uint32_t dispTmr;
static uint32_t strIdx;

void NetworkUpdat_displayCheck(void)
{
    if (HAL_GetElapsedMs(dispTmr)> 800){
        dispTmr = HAL_GetCurrentMsCount();
//        tubeDisplayStrSet(&strUpdating[strIdx&7]);
        strIdx++; 
        if (strIdx&0x01){
            GpioSet_LedRedSta(OUTPUT_HIGH);
            GpioSet_LedGreenSta(OUTPUT_LOW);
        }
        else{
            GpioSet_LedRedSta(OUTPUT_LOW);
            GpioSet_LedGreenSta(OUTPUT_HIGH);
        }
    }
}

long LIBM_xmodemReceive(long destsz)
{
    unsigned char *p;
    int bufsz, crc = 0;
    unsigned char trychar = 'C';
    unsigned char packetno = 1;
    long len = 0;
    int i, c;
    
    int retry, retrans = MAXRETRANS;
    xmode_printf ("Start recv bufsz\r\n");
    AppFwUpd_InitStart();
    strIdx = 0;
    dispTmr = HAL_GetCurrentMsCount();
    tubeDisplayStrSet(&strUpdating[strIdx&7]);
    strIdx++;
    DisableWDT();
    for(;;) {
        for( retry = 0; retry < 16; ++retry) {  // approx 100 seconds allowed to make connection
            if (trychar) 
                LIBM_UART_SET(trychar);
            if ((c = _checkByte(1000))) {
                switch (c) {
                case SOH:
                    bufsz = 128;
                    goto start_recv;
                case STX:
                    bufsz = 1024;
                    goto start_recv;
                case EOT:
                    _xflushIn();
                    LIBM_UART_SET(ACK);
                    GpioSet_LedRedSta(OUTPUT_HIGH);
                    GpioSet_LedGreenSta(OUTPUT_LOW);
                    xmode_printf ("xmodemReceive EOT\r\n");
                    return len; /* normal end */
                case CAN:
                    if ((c = _checkByte(1000)) == CAN) {
                        _xflushIn();
                        LIBM_UART_SET(ACK);
                        xmode_printf ("xmodemReceive CAN\r\n");
                        return -1; /* canceled by remote */
                    }
                    break;
                default:
                    break;
                }
            }
            else
            {
                xmode_printf ("xmodemReceive %d\r\n",c);
            }
        }
        if (trychar == 'C') { 
            trychar = NAK; 
            continue;
        }
        _xflushIn();
        LIBM_UART_SET(CAN);
        LIBM_UART_SET(CAN);
        LIBM_UART_SET(CAN);
        xmode_printf ("xmodemReceive NOack\r\n");
        return -2; /* sync error */

    start_recv:
        if (trychar == 'C') 
            crc = 1;
        trychar = 0;
        p = xbuff;
        *p++ = c;
        for (i = 0;  i < (bufsz+(crc?1:0)+3); ++i) {
            if ((c = _checkByte(1000)) < 0) 
                goto reject;
            *p++ = c;
        }

        if (xbuff[1] == (unsigned char)(~xbuff[2]) && 
            (xbuff[1] == packetno || xbuff[1] == (unsigned char)packetno-1) &&
                check(crc, &xbuff[3], bufsz)) {
            if (xbuff[1] == packetno)    
            {
                long count = destsz - len;
                if (count > bufsz) 
                    count = bufsz;
                if (count > 0) {
                    len += count;
                    //xmode_printf ("xmode read %d\r\n",count);
                    AppFwUpd_PackageDeliver(&xbuff[3], count);
                    NetworkUpdat_displayCheck();
                }
                ++packetno;
                retrans = MAXRETRANS+1;
            }
            if (--retrans <= 0) {
                _xflushIn();
                LIBM_UART_SET(CAN);
                LIBM_UART_SET(CAN);
                LIBM_UART_SET(CAN);
                xmode_printf ("xmodemReceive too many retry error\r\n");
                return -3; /* too many retry error */
            }
            LIBM_UART_SET(ACK);
            continue;
        }
    reject:
        _xflushIn();
        LIBM_UART_SET(NAK);
    }
    
    GpioSet_LedRedSta(OUTPUT_HIGH);
    GpioSet_LedGreenSta(OUTPUT_LOW);
}
    
#else

int main_xmodem(xmodeCbFunc cbFunc)
{
    return 0;
}

long LIBM_xmodemReceive(long destsz)
{
    return 0;
}

#endif


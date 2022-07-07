/***********************************************************************
* Copyright (c) 2016, Hansong (Nanjing) Technology Ltd.
* All rights reserved.
* 
* ----File Info ----------------------------------------------- -----------------------------------------
* Name:     hal_system_timer.c
* Author:   Baron.Peng
* Date:     2016-2-23
* Summary:  通用的字符串操作函数。
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
#include "com_assert.h"

void cmemcpy(void *data, void *source, unsigned long length)
{
    unsigned char *d = data;
    unsigned char *s = source;
    while(length)
    {
        *d = *s;
        if(length>1){
        s++;
        d++;}
        length--;
    }
}

char cstrncmp(void *cmp1x, void *cmp2x, unsigned long len)
{
    unsigned char *cmp1 = cmp1x;
    unsigned char *cmp2 = cmp2x;
    
    while(len>0)
    {
        if(*cmp1 != *cmp2)
            return 1;
        len--;
        if ((*cmp1 == '\0')||(*cmp2 == '\0')){
            if (len){
                return 1;
            }
            return 0;
        }
        cmp1++;
        cmp2++;
    }
    return 0;
}

char cmemncmp(void *cmp1x, void *cmp2x, unsigned long len)
{
    unsigned char *cmp1 = cmp1x;
    unsigned char *cmp2 = cmp2x;
    
    while(len>0)
    {
        if(*cmp1 != *cmp2)
            return 1;
        len--;
        cmp1++;
        cmp2++;
    }
    return 0;
}

char cstrcmp(void *cmp1x, void *cmp2x)
{
    unsigned char *cmp1 = cmp1x;
    unsigned char *cmp2 = cmp2x;
    
    do{
        if(*cmp1 != *cmp2)
            return 0;
        cmp1++;
        cmp2++;
    }while((*cmp1 != '\0')&&(*cmp2 != '\0'));

    if ((*cmp1 == '\0')&&(*cmp2 == '\0'))
        return 1;
    return 0;
}

char *cstrcopy(char *des, const char *source)
{

    char* r=des;

    //APP_ASSERT((des != NULL) && (source != NULL));
    if ((des == NULL) || (source == NULL))
        return des;
#if 0
    while ((*des++ = *source++)!='\0');
#else
    while (*source != '\0')
    {
        *des++ = *source++;
    }
    *des='\0';
#endif

    return r;
}

unsigned long cus_strlen(const char *src)
{
    unsigned long i;
    const char *p = src;
    for(i = 0; i < 32767; i++)
    {
        if(*p == '\0')
        {
            break;
        }
        p++;
    }
    return i;
}

unsigned char cstrsearch(char *source, const char *des, unsigned  char deslen)
{
    //
    //find des in source.
    unsigned int idx,chc,srclen;

    srclen = cus_strlen(source);

    if (srclen < deslen)
        return 0;
    idx = 0;
    chc = 0;

    while((srclen - idx) >= deslen){
        for (chc == 0 ; chc < deslen; chc++){
            if ((*(source + idx + chc)) != (*(des + chc)))
                break;
        }
        if (chc == deslen){
            return 1;
        }
        chc = 0;
        idx++;
    }
    return 0;
}

void cmemset(void *dir, unsigned char dat, unsigned int len)
{
    unsigned char *d = dir;
    while (len)
    {
        *d = dat;
        d++;
        len--;
    }
}

char  *cAsciiToHex (char  *Strg, unsigned char  *Value)
{
    unsigned char  Val=0, i;
    char *NumEnd;

    while (*Strg == ' ')
    {
        Strg++;
    }
    
    if (*Strg == '\0')
    {
        return ((char *)0);
    }
    
    if ((Strg[0] == '0') && (Strg[1] == 'x'))
    {
        Strg+= 2;
    }

    for (i=0; i<2; i++)
    {
        if ((Strg[i] >= '0') && (Strg[i] <= '9'))
        {
            Val <<= 4;
            Val |= (Strg[i] - '0');
        }
        else if ((Strg[i] >= 'a') && (Strg[i] <= 'f'))
        {
            Val <<= 4;
            Val |= (Strg[i] - 'a' + 10);
        }
        else if ((Strg[i] >= 'A') && (Strg[i] <= 'F'))
        {
            Val <<= 4;
            Val |= (Strg[i] - 'A' + 10);
        }
        else
        {
            break;
        }
    }
    
    if (i && ((Strg[i] == 0) || (Strg[i] == ' ') || (Strg[i] == ',')))
    {
        *Value = Val;
        if (Strg[i] == ',')
        {
            i++;
        }
        NumEnd = Strg+i;
        while (*NumEnd == ' ')
        {
            NumEnd++;
        }
        return (NumEnd);
    }
    return ((char *)0);
}

char  *cAsciiTo16bitHex(char  *Strgx, unsigned short int  *Value)
{
    unsigned char  i;
    char *Strg = Strgx;
    unsigned short Val = 0;
    unsigned short tmp;

    while (*Strg == ' ')
    {
        Strg++;
    }
    
    if ((Strg[0] == '0') && (Strg[1] == 'x'))
    {
        Strg+= 2;
    }

    for (i=0; i<4; i++)
    {
        if ((Strg[i] >= '0') && (Strg[i] <= '9'))
        {
            Val <<= 4;
            tmp = (Strg[i] - '0')&0x00F;
            Val |= tmp;
        }
        else if ((Strg[i] >= 'a') && (Strg[i] <= 'f'))
        {
            Val <<= 4;
            tmp = (Strg[i] - 'a' + 10)&0x00F;
            Val |= tmp;
        }
        else if ((Strg[i] >= 'A') && (Strg[i] <= 'F'))
        {
            Val <<= 4;
            tmp = (Strg[i] - 'A' + 10)&0x00F;
            Val |= tmp;
        }
        else
        {
            break;
        }
    }
    
    if (i && ((Strg[i] == '\0') || (Strg[i] == ' ') || (Strg[i] == ',')))
    {
        *Value = Val;
        if (Strg[i] == ',')
        {
            i++;
        }
        Strg = Strg+i;
        while (*Strg == ' ')
        {
            Strg++;
        }
        return (Strg);
    }
    return ((char *)0);

}

char  *cAsciiTo32bitHex(char  *Strgx, unsigned int  *Value)
{
    unsigned char  i;
    char *Strg = Strgx;
    unsigned long Val = 0;
    unsigned long tmp;

    while (*Strg == ' ')
    {
        Strg++;
    }
    
    if ((Strg[0] == '0') && (Strg[1] == 'x'))
    {
        Strg+= 2;
    }

    for (i=0; i<8; i++)
    {
        if ((Strg[i] >= '0') && (Strg[i] <= '9'))
        {
            Val <<= 4;
            tmp = (Strg[i] - '0')&0x00F;
            Val |= tmp;
        }
        else if ((Strg[i] >= 'a') && (Strg[i] <= 'f'))
        {
            Val <<= 4;
            tmp = (Strg[i] - 'a' + 10)&0x00F;
            Val |= tmp;
        }
        else if ((Strg[i] >= 'A') && (Strg[i] <= 'F'))
        {
            Val <<= 4;
            tmp = (Strg[i] - 'A' + 10)&0x00F;
            Val |= tmp;
        }
        else
        {
            break;
        }
    }
    
    if (i && ((Strg[i] == '\0') || (Strg[i] == ' ') || (Strg[i] == ',')))
    {
        *Value = Val;
        if (Strg[i] == ',')
        {
            i++;
        }
        Strg = Strg+i;
        while (*Strg == ' ')
        {
            Strg++;
        }
        return (Strg);
    }
    return ((char *)0);

}

unsigned char  cHex2String (unsigned char  chex, unsigned char *cchar)
{
    unsigned char val = 0;
    val = (chex>>4)&0x0F;
    if ((val >= '0')&&(val <= '9'))
    {
        *cchar = (val- '0')<< 4;
    }
    else
    {
        return 0;
    }
    val = chex&0x0F;
    if ((val >= '0')&&(val <= '9'))
    {
        *cchar += (val- '0');
    }
    return (0);
}


#ifndef _LIB_XMODE_H_
#define _LIB_XMODE_H_

//unsigned char DSP__BufferProgramming_xmode(unsigned char *dat, unsigned long datlen);
typedef unsigned char (*xmodeCbFunc)(unsigned char *dat,unsigned int datlen);

extern int main_xmodem(xmodeCbFunc cbFunc);

#endif



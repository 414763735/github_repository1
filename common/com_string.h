#ifndef _COMMON_STRING_H_
#define _COMMON_STRING_H_

extern void          cmemset(void *dir, unsigned char dat, unsigned int len);
extern void          cmemcpy(void *data, void *source, unsigned long length);
extern char          cmemncmp(void *cmp1x, void *cmp2x, unsigned long len);
extern char          cstrncmp(void *cmp1x, void *cmp2x, unsigned long len);
extern char          cstrcmp(void *cmp1x, void *cmp2x);
extern char         *cstrcopy(char *des, const char *source);
extern unsigned long cus_strlen(const char *src);
extern char         *cAsciiToHex (char  *Strg, unsigned char  *Value);
extern char         *cAsciiTo16bitHex(char  *Strg, unsigned short int  *Value);
extern char         *cAsciiTo32bitHex(char  *Strg, unsigned       int  *Value);
extern unsigned char cHex2String (unsigned char  chex, unsigned char *cchar);
extern unsigned char cstrsearch(char *source, const char *des, unsigned  char deslen);

#endif


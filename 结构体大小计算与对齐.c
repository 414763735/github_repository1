/*****************************************************************************
Copyright: NULL
File name: strFuncTest.c
Description: 用于熟悉掌握结构体的对齐以及大小计算方法
Author: 胡凯旋
Version: 1.0
Date: 2022/08/10
History: NULL
*****************************************************************************/

#include<stdio.h>

typedef struct
{
    int a;
    char b;
    short c;

}A;
/* A = 4
 * 计算结构体大小两大原则:
 * 1，下一个成员占用长度要整除上一个成员占用字节数，否则上一个成员补齐
 * 2，结构体整体大小一定是最长的成员字节数的倍数
 */

typedef struct 
{
    char m;
    short n;
    A a;
}B;
/* 计算 B 的大小时只需要计算没有 a 时的大小 + a 
 * 即 4 + 8 = 12
 */

int main()
{
    printf("A = %d,B = %d\r\n",sizeof(A), sizeof(B));
    return 0;
}

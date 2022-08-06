/*****************************************************************************
Copyright: NULL
File name: strFuncTest.c
Description: 用于介绍测试字符串操作函数，学习常用字符串的操作函数 
Author: 胡凯旋 
Version: 1.0
Date: 2022/08/06
History: NULL
*****************************************************************************/
#include<string.h>
#include<stdio.h>
#include <stdlib.h>
int main() {
	char str[20] = "hu_kai_xuan";
	char temp[20] = "hello_world";
	printf("temp = %s\r\n", temp);
	
	memset(temp, 0, sizeof(temp));//把字符'0'写入 sizeof(temp)个字节到temp
	printf("temp = %s\r\n", temp);
	
	memcpy(temp, str, strlen(str));//从存储区 str 复制 strlen(str) 个字节到存储区 temp
	printf("strlen of str = %d\r\n", strlen(str));//strlen(str)计算str的长度'\0'结束
	printf("size of str = %d\r\n", sizeof(str));//sizeof(str)计算str初始化时申请的内存大小
	printf("temp = %s\r\n", temp);

	char *p = (char*)malloc(sizeof(temp));//申请内存,用多少申请多少,避免像数组一样造成内存浪费
	strcpy(p, temp);//拷贝字符串到指定地址 
	printf("p = %s\r\n", p);
	
	char *token;
	token = strtok(p, "_");//分割字符串，以"k"分隔符获取分割后的字符串
	while(token != NULL){
		printf("token = %s\r\n", token);
		/*
		第一次调用时，strtok函数的第一个参数传入要分割的字符串，而第二次以及后面再次调用该函数的时候，
		strtok函数的第一个参数应该传入NULL，这是因为在strtok第一个参数为NULL的时候，
		该函数默认使用上一次未分割完的字符串的未分割的起始位置作为本次分割的起始位置，直到分割结束为止。
		*/
		token = strtok(NULL, "_");
	}
	
	system("pause");
	
}

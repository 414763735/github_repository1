#include<string.h>
#include<stdio.h>
#include <stdlib.h>
int main() {
	char str[20] = "hukaixuan";
	char temp[20] = {0};

	memcpy(temp, str, strlen(str));//从存储区 str 复制 strlen(str) 个字节到存储区 temp
	printf("strlen of str = %d\r\n", strlen(str));//strlen(str)计算str的长度'\0'结束
	printf("size of str = %d\r\n", sizeof(str));//sizeof(str)计算str初始化时申请的内存大小
	printf("temp = %s\r\n", temp);
	
	memset(temp, 0, sizeof(temp));//把字符'0'写入 sizeof(temp)个字节到temp
	printf("temp = %s\r\n", temp);
	
	char *p = (char*)malloc(sizeof(temp));//申请内存,用多少申请多少,避免像数组一样造成内存浪费
	strcpy(p, temp);//拷贝字符串到指定地址 
	printf("p = %s\r\n", p);
	
	system("pause");
	
}

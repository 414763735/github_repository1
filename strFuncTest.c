/*****************************************************************************
Copyright: NULL
File name: strFuncTest.c
Description: ���ڽ��ܲ����ַ�������������ѧϰ�����ַ����Ĳ������� 
Author: ������ 
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
	
	memset(temp, 0, sizeof(temp));//���ַ�'0'д�� sizeof(temp)���ֽڵ�temp
	printf("temp = %s\r\n", temp);
	
	memcpy(temp, str, strlen(str));//�Ӵ洢�� str ���� strlen(str) ���ֽڵ��洢�� temp
	printf("strlen of str = %d\r\n", strlen(str));//strlen(str)����str�ĳ���'\0'����
	printf("size of str = %d\r\n", sizeof(str));//sizeof(str)����str��ʼ��ʱ������ڴ��С
	printf("temp = %s\r\n", temp);

	char *p = (char*)malloc(sizeof(temp));//�����ڴ�,�ö����������,����������һ������ڴ��˷�
	strcpy(p, temp);//�����ַ�����ָ����ַ 
	printf("p = %s\r\n", p);
	
	char *token;
	token = strtok(p, "_");//�ָ��ַ�������"k"�ָ�����ȡ�ָ����ַ���
	while(token != NULL){
		printf("token = %s\r\n", token);
		/*
		��һ�ε���ʱ��strtok�����ĵ�һ����������Ҫ�ָ���ַ��������ڶ����Լ������ٴε��øú�����ʱ��
		strtok�����ĵ�һ������Ӧ�ô���NULL��������Ϊ��strtok��һ������ΪNULL��ʱ��
		�ú���Ĭ��ʹ����һ��δ�ָ�����ַ�����δ�ָ����ʼλ����Ϊ���ηָ����ʼλ�ã�ֱ���ָ����Ϊֹ��
		*/
		token = strtok(NULL, "_");
	}
	
	system("pause");
	
}

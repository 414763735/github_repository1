#include<string.h>
#include<stdio.h>
#include <stdlib.h>
int main() {
	char str[20] = "hukaixuan";
	char temp[20] = {0};

	memcpy(temp, str, strlen(str));//�Ӵ洢�� str ���� strlen(str) ���ֽڵ��洢�� temp
	printf("strlen of str = %d\r\n", strlen(str));//strlen(str)����str�ĳ���'\0'����
	printf("size of str = %d\r\n", sizeof(str));//sizeof(str)����str��ʼ��ʱ������ڴ��С
	printf("temp = %s\r\n", temp);
	
	memset(temp, 0, sizeof(temp));//���ַ�'0'д�� sizeof(temp)���ֽڵ�temp
	printf("temp = %s\r\n", temp);
	
	char *p = (char*)malloc(sizeof(temp));//�����ڴ�,�ö����������,����������һ������ڴ��˷�
	strcpy(p, temp);//�����ַ�����ָ����ַ 
	printf("p = %s\r\n", p);
	
	system("pause");
	
}

// Win-�޸���ڵ���ת.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<cstdio>
#include<windows.h>
int PEHeadOffset = 0;//PEͷ������ļ���ƫ��
const int PEtoEntryPoint = 0x28;//��ڵ������PEͷ��ƫ��
int EntryPoint = 0;//ԭ��ڵ�
int textVK = 0;//��¼text����RVA��File Offset�Ĳ�ֵ��File Offset=RVA-VK

int findSection(char *name, PVOID file, int start)//�ҵ�PEͷ������Ϊ*name�����ε�������Ϣ���ļ�ƫ��
{
	//int offset = start;
	bool OK;
	while (1)
	{
		OK = true;
		for (int i = 0;; i++)
		{
			if (*(name + i) == '\0'&&*((PSTR)file + i + start) == '\0')
				break;
			if (*(name + i) != *((PSTR)file + i + start))
			{
				OK = false;
				break;
			}
		}
		if (OK)
		{
			return start;
		}
		else
		{
			start++;
		}
	}
}
int findZero(PVOID file,int start)//����һ������128�ֽڶ���0������
{
	bool OK = true;
	while (1)
	{
		OK = true;
		for (int i = 0; i < 128; i++)
		{
			if (0 != *((PSTR)file + start +i))
			{
				OK = false;
				break;
			}
		}
		if (OK)
		{
			return start;
		}
		else
		{
			start++;
		}
	}
}
int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR szFileAddress[512];
	wscanf_s(L"%ls", szFileAddress,_countof(szFileAddress));
	HANDLE hNormalFile = CreateFile(szFileAddress, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hNormalFile == INVALID_HANDLE_VALUE)
	{
		printf("���ļ�ʧ��\n");
	}
	DWORD dwFileSize = GetFileSize(hNormalFile, NULL);
	HANDLE hFileMap = CreateFileMapping(hNormalFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
	if (hFileMap == NULL)
	{
		printf("�����ڴ�ӳ���ļ�ʧ��\n");
	}
	PVOID pvFile = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == pvFile)
	{
		printf("ӳ���ļ�ʧ��\n");
	}
	
	PEHeadOffset = *(int *)((PSTR)pvFile + 0x3c);
	EntryPoint = *(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint);
	//PEͷƫ��0x78��DataDirectory�ֶΣ�DataDirectory�ֶΰ���16����Ϣ��ÿ��8�ֽ�
	//textVK = *(int *)((PSTR)pvFile + PEHeadOffset + 0x78+16*8+)
	int textOffset = findSection(".text", pvFile,0);
	int textRVA = *(int *)((PSTR)pvFile + textOffset + 12);//���α�����ʼƫ��12�ֽڴ�Ϊ�����ε�RVA
	int textFO = *(int *)((PSTR)pvFile + textOffset + 20);//���α�����ʼƫ��20�ֽڴ�Ϊ�����ε�File Offset
	int textSize = *(int *)((PSTR)pvFile + textOffset + 16);//ƫ��16�ֽ�Ϊ���������ļ��ж����ĳߴ�
	textVK = textRVA - textFO;
	int zeroOffset = findZero(pvFile, textFO);
	
	if (zeroOffset < textFO || zeroOffset >= textFO + textSize - 128)
	{
		printf(".text����δ�ҵ�128�ֽڵĿհ�����");
		return 1;
	}

	*((PSTR)pvFile + zeroOffset) = 0xe9;
	*(int *)((PSTR)pvFile + zeroOffset + 1) = EntryPoint - (zeroOffset + textVK) - 5;//zeroOffsetΪ�ļ�ƫ�ƣ��������ת��RVA

	*(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint) = zeroOffset + textVK;

	UnmapViewOfFile(pvFile);
	return 0;
}


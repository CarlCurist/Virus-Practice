// Win-修改入口点跳转.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<cstdio>
#include<windows.h>
int PEHeadOffset = 0;//PE头相对于文件的偏移
const int PEtoEntryPoint = 0x28;//入口点相对于PE头的偏移
int EntryPoint = 0;//原入口点
int textVK = 0;//记录text区段RVA与File Offset的差值。File Offset=RVA-VK

int findSection(char *name, PVOID file, int start)//找到PE头中名字为*name的区段的描述信息的文件偏移
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
int findZero(PVOID file,int start)//查找一块连续128字节都是0的区域
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
		printf("打开文件失败\n");
	}
	DWORD dwFileSize = GetFileSize(hNormalFile, NULL);
	HANDLE hFileMap = CreateFileMapping(hNormalFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
	if (hFileMap == NULL)
	{
		printf("创建内存映射文件失败\n");
	}
	PVOID pvFile = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == pvFile)
	{
		printf("映射文件失败\n");
	}
	
	PEHeadOffset = *(int *)((PSTR)pvFile + 0x3c);
	EntryPoint = *(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint);
	//PE头偏移0x78是DataDirectory字段，DataDirectory字段包含16条信息，每条8字节
	//textVK = *(int *)((PSTR)pvFile + PEHeadOffset + 0x78+16*8+)
	int textOffset = findSection(".text", pvFile,0);
	int textRVA = *(int *)((PSTR)pvFile + textOffset + 12);//区段表中起始偏移12字节处为该区段的RVA
	int textFO = *(int *)((PSTR)pvFile + textOffset + 20);//区段表中起始偏移20字节处为该区段的File Offset
	int textSize = *(int *)((PSTR)pvFile + textOffset + 16);//偏移16字节为该区段在文件中对齐后的尺寸
	textVK = textRVA - textFO;
	int zeroOffset = findZero(pvFile, textFO);
	
	if (zeroOffset < textFO || zeroOffset >= textFO + textSize - 128)
	{
		printf(".text区段未找到128字节的空白区域");
		return 1;
	}

	*((PSTR)pvFile + zeroOffset) = 0xe9;
	*(int *)((PSTR)pvFile + zeroOffset + 1) = EntryPoint - (zeroOffset + textVK) - 5;//zeroOffset为文件偏移，必须把它转成RVA

	*(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint) = zeroOffset + textVK;

	UnmapViewOfFile(pvFile);
	return 0;
}


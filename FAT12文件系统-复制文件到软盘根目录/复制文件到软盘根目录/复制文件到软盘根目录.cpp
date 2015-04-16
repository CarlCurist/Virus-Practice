// �����ļ������̸�Ŀ¼.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include<cstdio>
#include<windows.h>

BYTE RootNum = 0;
#define FATOffset 512
#define RootDirectoryOffset 9728
#define DataOffset 9728+RootNum*32
#define FLOPPY_SIZE 1474560
char RootValue[] = { 0x35, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x44, 0x41, 0x54, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
char content1[] = { 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
char content2[] = { 0x2E, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
int Sector[100], totalSector = 0;

unsigned long getFileSize(TCHAR *FileAddr)
{
	HANDLE hFile = CreateFile(FileAddr, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("���ļ�����\n������룺%d\n",GetLastError());
		return -1;
	}
	unsigned long FileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return FileSize;
}


int WriteCluster(FILE *Floppy, int SectorNum,int writeValue)
{
	rewind(Floppy);
	int offset = FATOffset + (SectorNum / 2) * 3;
	DWORD SingleItem, SecondByte;
	BYTE item[3];

	fseek(Floppy, offset, SEEK_SET);
	fread(item, 3, sizeof(BYTE), Floppy);
	if (SectorNum % 2)//�ڶ���
	{
		SingleItem = item[2];
		SecondByte = item[1];
		SingleItem = SingleItem << 4;
		SecondByte = SecondByte >> 4;
		SingleItem |= SecondByte;

		fseek(Floppy, -3, SEEK_CUR);
		item[1] &= 15;//15������1111����������԰�ǰ��ر��0
		item[1] |= ((writeValue % (1 << 4))*(1 << 4));
		item[2] = writeValue / (1 << 4);
		fwrite(item, 3, sizeof(BYTE), Floppy);
		return 0;
	}
	else//��һ��
	{
		SingleItem = item[0];
		SecondByte = item[1] % (1 << 4);
		SecondByte = SecondByte << 8;
		SingleItem |= SecondByte;

		fseek(Floppy, -3, SEEK_CUR);
		item[1] &= 240;//240������Ϊ1111 0000����������԰Ѻ��ر��0
		item[1] |= (writeValue / (1 << 8));
		item[0] = writeValue % (1 << 8);
		fwrite(item, 3, sizeof(BYTE), Floppy);
		return 0;
	}
}

int FindAvailableSector(FILE *Floppy)
{
	BYTE szBuf[512];
	int SectorNum=2;
	rewind(Floppy);
	fseek(Floppy, DataOffset, SEEK_SET);
	for (;;SectorNum++)
	{
		fread(szBuf, 512, sizeof(BYTE), Floppy);
		/*
		if (512 != fread(szBuf, 512, sizeof(BYTE), Floppy))
		{
			return -1;
		}
		*/
		if (szBuf[0] == 0x00 || szBuf[0] == 0xe5 || szBuf[0] == 0xf6)
		{
			Sector[totalSector++] = SectorNum;
			return SectorNum;
		}
	}
	
}

int copyFile(FILE *Floppy,FILE *File,int SectorNum,int FileStartOffset,int writeLong)
{
	BYTE szBuf[512];
	int offset = DataOffset + 512 * (SectorNum - 2);
	rewind(Floppy);
	fseek(File, FileStartOffset, SEEK_SET);
	fread(szBuf, writeLong, sizeof(BYTE), File);

	fseek(Floppy, offset, SEEK_SET);
	fwrite(szBuf, writeLong, sizeof(BYTE), Floppy);
	return 0;
}

int writeRootValue(FILE *Floppy, int fileSize, int firstCluster, char *fileName,int StartOffset)
{
	BYTE szBuf[32],tempByte=0x20,tempByte2=0;
	fseek(Floppy, StartOffset, SEEK_SET);
	/*
	bool arrivePoint = false;
	for (int i = 0, len = wcslen(fileName), current = 0; i < 26; i++)//��26�ֽڴ��Ϳ�ʼд�״غʹ�С
	{
		if (i >= 11)
		{
			fwrite(&tempByte2, 1, sizeof(BYTE), Floppy);
			continue;
		}
		
		if (i < 11 && arrivePoint)
		{
			fwrite(&tempByte, 1, sizeof(BYTE), Floppy);
			continue;
		}
		if ('.' == fileName[i])
		{
			arrivePoint = true;
			current++;
			continue;
		}
		fwrite(&fileName[current++], 1, sizeof(BYTE), Floppy);
	}
	*/
	int a,b;
	while (1)
	{
		fread(szBuf, 32, sizeof(BYTE), Floppy);
		if (szBuf[0] == 0 || szBuf[0] == 0xf6)
		{
			fseek(Floppy, -32, SEEK_CUR);
			bool arrivePoint = false;
			for (int i = 0, len = strlen(fileName), current = 0; i < 26; i++)//��26�ֽڴ��Ϳ�ʼд�״غʹ�С
			{

				if (i > 11)
				{
					fseek(Floppy, 0, SEEK_CUR);
					fwrite(&tempByte2, 1, sizeof(BYTE), Floppy);
					continue;
				}

				if (i < 8 && arrivePoint)
				{
					fseek(Floppy, 0, SEEK_CUR);
					fwrite(&tempByte, 1, sizeof(BYTE), Floppy);
					continue;
				}
				if ('.' == fileName[i] && i < len)
				{
					arrivePoint = true;
					fwrite(&tempByte, 1, sizeof(BYTE), Floppy);
					current++;
					continue;
				}
				fseek(Floppy, 0, SEEK_CUR);
				BYTE byte = fileName[current++];
				if (byte >= 'a'&&byte <= 'z')
					byte -= 32;//��Сдת���ɴ�д����Ϊ��չ��ֻ֧�ִ�д
				a = fwrite(&byte, 1, sizeof(BYTE), Floppy);
				b = ferror(Floppy);
			}

			///////////////////����test
			//fseek(Floppy, -32, SEEK_CUR);
			//fwrite(RootValue, sizeof(RootValue), sizeof(BYTE), Floppy);
			///////////////д�״�
			tempByte = firstCluster % (1 << 8);
			fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);

			tempByte = firstCluster / (1 << 8);
			fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);
			////////////////////

			///////////////////д��С
			for (int i = 0; i < 4;i++)
			{
				tempByte = fileSize % (1 << 8);
				fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);
				fileSize /= 1 << 8;
			}
			return 0;
		}
	}

}
int writeFile(FILE *Floppy, TCHAR *FileAddr, char *FileName, int StartOffset)
{
	FILE *File;
	unsigned long FileSize;
	int NeedSectorNum, SectorNum;
	totalSector = 0;
	TCHAR szFileAddr[512];


	FileSize = getFileSize(FileAddr);//��ȡ�ļ���С
	NeedSectorNum = (FileSize % 512 == 0) ? FileSize / 512 : FileSize / 512 + 1;//������Ҫ����������

	errno_t err2 = _wfopen_s(&File, FileAddr, L"rb+");

	if (FileSize > FLOPPY_SIZE)
	{
		printf("�ļ�����\n");
		return 0;
	}

	int remainDataLong = FileSize;//ʣ��û���Ƶ��ļ�����
	for (int i = NeedSectorNum; i > 0; i--)
	{

		SectorNum = FindAvailableSector(Floppy);
		copyFile(Floppy, File, SectorNum, (NeedSectorNum - i) * 512, (i == 1) ? remainDataLong : 512);
		remainDataLong -= 512;
	}
	for (int i = 0; i <NeedSectorNum; i++)
	{
		if (i != NeedSectorNum - 1)
			WriteCluster(Floppy, Sector[i], Sector[i + 1]);
		else
			WriteCluster(Floppy, Sector[i], 0xfff);
	}

	//writeRootValue(Floppy, FileSize, Sector[0]);
	writeRootValue(Floppy, FileSize, Sector[0], FileName, StartOffset);
	
	fclose(File);
	return 0;
}
int CreateContents(FILE *Floppy)
{
	int NeedSectorNum, SectorNum;
	totalSector = 0;
	TCHAR szFileAddr[512];
	BYTE tempByte;

	SectorNum = FindAvailableSector(Floppy);
	
	WriteCluster(Floppy, SectorNum, 0xfff);

	fseek(Floppy, DataOffset + (SectorNum-2) * 512, SEEK_SET);
	fwrite(content1, sizeof(content1), sizeof(BYTE), Floppy);

	///////////////д�״�
	tempByte = SectorNum % (1 << 8);
	fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);

	tempByte = SectorNum / (1 << 8);
	fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);
	////////////////////

	BYTE t2 = 0;
	for (int i = 0; i < 4;i++)
		fwrite(&t2, 1, 1, Floppy);//��ΪĿ¼û�д�С��������0���

	fwrite(content2, sizeof(content2), sizeof(BYTE), Floppy);

	char s[] = { "TESTTEST   " };
	BYTE t = 0x10;
	writeRootValue(Floppy, 0, SectorNum, s, RootDirectoryOffset);
	fseek(Floppy, RootDirectoryOffset +32+11, SEEK_SET);
	fwrite(&t, 1, 1, Floppy);//�Ѹ�Ŀ¼����ĳ�0x10����ʾ����һ��Ŀ¼

	//writeRootValue(Floppy, FileSize, Sector[0]);
	//writeRootValue(Floppy, FileSize, Sector[0], FileName, RootDirectoryOffset);

	return SectorNum;
}
int Combine(TCHAR *s1, TCHAR *s2)
{
	int s1long = wcslen(s1);
	int s2long = wcslen(s2);
	s1[s1long - 1] = 0;
	s1[s1long - 2] = 0;
	//s1[s1long - 3] = '\\';

	int i, j;
	for (i = s1long - 3, j = 0; j < s2long; j++, i++)
	{
		s1[i] = s2[j];
	}
	s1[i] = 0;
	return 0;
}
int _tmain(int argc, _TCHAR* argv[])
{
	FILE *Floppy,*File;
	TCHAR FloppyAddr[512],FileAddr[512],FileAddrCopy[512];
	//////////////////////
	memset(FloppyAddr, 0, sizeof(FloppyAddr));
	memset(FileAddr, 0, sizeof(FileAddr));
	memset(FileAddrCopy, 0, sizeof(FileAddrCopy));
	///////////////////////
	BYTE szBuf[512],btemp;
	unsigned long FileSize;
	DWORD btemp2;
	int NeedSectorNum,SectorNum;
	printf("�������̵�ַ\n");
	wscanf_s(L"%ls", FloppyAddr, _countof(FloppyAddr));
	printf("���뱻���Ƶ�Ŀ¼����\*.*��β��\n");
	wscanf_s(L"%ls", FileAddr, _countof(FileAddr));
	
	wcscpy_s(FileAddrCopy, _countof(FileAddrCopy), FileAddr);

	errno_t err = _wfopen_s(&Floppy, FloppyAddr, L"rb+");
	//errno_t err2 = fopen_s(&File, FileAddr, "rb+");

/////////////////////////��ȡ��Ŀ¼�������
	fseek(Floppy, 17, SEEK_SET);
	fread(&RootNum, 1, sizeof(BYTE), Floppy);
	rewind(Floppy);
/////////////////////////

	int Sector1 = CreateContents(Floppy);
	WIN32_FIND_DATA p;
	HANDLE h = FindFirstFile(FileAddr, &p);
	int i = 0;
	while (FindNextFile(h, &p))
	{
		if (p.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)
			continue;
		memset(FileAddrCopy, 0, sizeof(FileAddrCopy));
		wcscpy_s(FileAddrCopy, _countof(FileAddrCopy), FileAddr);
		Combine(FileAddrCopy, p.cFileName);
		

		/////////////////unicode to char
		//wchar_t wText[20] = { L"���ַ�ת��ʵ��!OK!" };
		DWORD dwNum = WideCharToMultiByte(CP_OEMCP, NULL, p.cFileName, -1, NULL, 0, NULL, FALSE);
		char psText[512];
		WideCharToMultiByte(CP_OEMCP, NULL, p.cFileName, -1, psText, dwNum, NULL, FALSE);
		
		////////////////////

		writeFile(Floppy, FileAddrCopy, psText, DataOffset+512*(Sector1-2)+64+i*32);

		i++;
		//writeFile(Floppy, FileAddrCopy, psText, RootDirectoryOffset);

	}

	return 0;
}


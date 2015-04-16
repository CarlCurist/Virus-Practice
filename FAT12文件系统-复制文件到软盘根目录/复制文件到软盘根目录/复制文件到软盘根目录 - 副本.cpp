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
int Sector[100], totalSector = 0;

unsigned long getFileSize(char *FileAddr)
{
	HANDLE hFile = CreateFileA(FileAddr, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		printf("���ļ�����\n������룺%d\n",GetLastError());
		return -1;
	}
	unsigned long FileSize = GetFileSize(hFile, NULL);
	CloseHandle(hFile);
	return FileSize;
}

/*
int FindAvailableCluster(FILE *Floppy, int SectorNum)
{
	rewind(Floppy);
	int offset = FATOffset;
	DWORD SingleItem, SecondByte;
	BYTE item[3];
	fseek(Floppy, FATOffset, SEEK_SET);
	for (;;)
	{
		fread(&RootNum, 3, sizeof(BYTE), Floppy);//���ò���д�
		
		////////////////////////��ȡ��һ��FAT��
		SingleItem = item[0];
		SecondByte = item[1] % (1 << 4);
		SecondByte = SecondByte << 8;
		SingleItem |= SecondByte;
		//SecondByte = item[1];
		//SecondByte %= 1 << 4;
		//SingleItem = SingleItem * 0x10 + SecondByte;
		/////////////////////////�����һ��Ϊ�գ�д���һ��
		if (SingleItem == 0)
		{
			fseek(Floppy, -3, SEEK_CUR);
			item[1] &= 240;//240������Ϊ1111 0000����������԰Ѻ��ر��0
			item[1] |= (SectorNum / (1 << 8));
			item[0] = SectorNum % (1 << 8);
			fwrite(item, 3, sizeof(BYTE), Floppy);
			return 0;
		}
		//////////////////////


			///////////////////////��ȡ�ڶ���FAT��
		SingleItem = item[2];
		SecondByte = item[1];
		SingleItem = SingleItem << 4;
		SecondByte = SecondByte >> 4;
		SingleItem |= SecondByte;
		//////////////////////////
		if (SingleItem == 0)
		{
			fseek(Floppy, -3, SEEK_CUR);
			item[1] &= 15;//15������1111����������԰�ǰ��ر��0
			item[1] |= ((SectorNum % (1 << 4))*(1 << 4));
			item[2] = SectorNum / (1 << 4);
			fwrite(item, 3, sizeof(BYTE), Floppy);
			return 0;

		}


	}
}
*/
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

int writeRootValue(FILE *Floppy, int fileSize, int firstCluster)
{
	BYTE szBuf[32],tempByte;
	fseek(Floppy, RootDirectoryOffset, SEEK_SET);
	while (1)
	{
		fread(szBuf, 32, sizeof(BYTE), Floppy);
		if (szBuf[0] == 0)
		{
			fseek(Floppy, -32, SEEK_CUR);
			fwrite(RootValue, sizeof(RootValue), sizeof(BYTE), Floppy);
			///////////////д�״�
			tempByte = firstCluster % (1 << 8);
			fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);

			tempByte = firstCluster / (1 << 8);
			fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);
			////////////////////

			///////////////////д��С
			while (fileSize != 0)
			{
				tempByte = fileSize % (1 << 8);
				fwrite(&tempByte, sizeof(tempByte), sizeof(BYTE), Floppy);
				fileSize /= 1 << 8;
			}
			return 0;
		}
	}

}
int _tmain(int argc, _TCHAR* argv[])
{
	FILE *Floppy,*File;
	char FloppyAddr[512],FileAddr[512];
	BYTE szBuf[512],btemp;
	unsigned long FileSize;
	DWORD btemp2;
	int NeedSectorNum,SectorNum;
	printf("�������̵�ַ\n");
	scanf_s("%s", FloppyAddr, _countof(FloppyAddr));
	printf("���뱻���Ƶ��ļ���ַ\n");
	scanf_s("%s", FileAddr, _countof(FileAddr));
	FileSize = getFileSize(FileAddr);//��ȡ�ļ���С
	NeedSectorNum = (FileSize % 512 == 0) ? FileSize / 512 : FileSize / 512 + 1;//������Ҫ����������
	if (FileSize > FLOPPY_SIZE)
	{
		printf("�ļ�����\n");
		return 0;
	}
	//Floppy = fopen(FloppyAddr, "rb");
	errno_t err = fopen_s(&Floppy, FloppyAddr, "rb+");
	errno_t err2 = fopen_s(&File, FileAddr, "rb+");

/////////////////////////��ȡ��Ŀ¼�������
	fseek(Floppy, 17, SEEK_SET);
	fread(&RootNum, 1, sizeof(BYTE), Floppy);
	rewind(Floppy);
/////////////////////////

	int remainDataLong = FileSize;//ʣ��û���Ƶ��ļ�����
	for (int i=NeedSectorNum; i > 0; i--)
	{

		SectorNum = FindAvailableSector(Floppy);
		//FindAvailableCluster(Floppy, SectorNum);
		copyFile(Floppy, File, SectorNum, (NeedSectorNum - i) * 512,(i==1)?remainDataLong:512);
		remainDataLong -= 512;
		/*
		rewind(Floppy);
		fread(szBuf, 512, sizeof(BYTE), File);
		fseek(Floppy, DataOffset+(SectorNum-2)*512, SEEK_SET);
		fwrite(szBuf, 512, sizeof(BYTE), Floppy);
		*/
	}
	for (int i = 0; i <NeedSectorNum; i++)
	{
		if (i != NeedSectorNum - 1)
			WriteCluster(Floppy, Sector[i], Sector[i + 1]);
		else
			WriteCluster(Floppy, Sector[i], 0xfff);
	}

	writeRootValue(Floppy, FileSize, Sector[0]);


	/*FAT 3�ֽڵ�һ��
	btemp2 = 0xFF;
	fread(&btemp, 1, 1, Floppy);
	//btemp = btemp << 4;
	//btemp /= 0x10;
	btemp = btemp / (1 << 4);
	btemp2 += (btemp)*0x100;
	//printf("%X", btemp);
	printf("%X", btemp2);
	*/

	/*FAT 3�ֽڵڶ���
	btemp2 = 0xFF;
	fread(&btemp, 1, 1, Floppy);
	//btemp = btemp >> 4;
	btemp = btemp % (1 << 4);
	btemp2 *= 0x10;
	btemp2 += btemp;
	printf("%X", btemp2);
	*/

	//fseek(Floppy, 1, SEEK_SET);

	/*
	fread(szBuf, 511, sizeof(BYTE), Floppy);
	
	for (int a = 0; a < 512; a++)
	{
		printf("%X ", szBuf[a]);
	}
	*/
	return 0;
}


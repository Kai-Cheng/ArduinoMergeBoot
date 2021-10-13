// Main.cpp
//

#include <iostream>
#include <windows.h>

int getLineCount(char* FilePath)
{
	FILE *SFile = NULL;
	errno_t Err = 0;
	int LineCount = 1;
	int ColNumber = 0;
	int PosNumber = 0;
	int Char1;

	Err = fopen_s(&SFile, FilePath, "rb");
	if (Err) {
		printf("can't open %s.", FilePath);
		return 0;
	}
	while ((Char1 = getc(SFile)) != EOF)
	{
		if (Char1 == 0x0A)
		{
			ColNumber = 0;
			++LineCount;
		}
		else
			++ColNumber;
		++PosNumber;
	}
	//printf("Line: %d, Col: %d, Pos: %d\r\n", LineCount, ColNumber, PosNumber);
	fclose(SFile);
	return LineCount;
}

int main(int argc, char *argv[])
{
	FILE *SFile = NULL;
	FILE *DestFile = NULL;
	int BootFileCount = 0;
	int FirmFileCount = 0;
	int DateFlag = 0;
	int TimeFlag = 0;
	int LineCount = 1;
	int DestLineCount = 1;
	int DestFileSize = 0;
	int Char1;
	char NewFileName[MAX_PATH];
	SYSTEMTIME lt;
	errno_t Err = 0;
	//unsigned char version_main = 0, version_sub = 0, version_test = 0;

#if 0//test
	argc = 4;
	argv[0] = reinterpret_cast <_TCHAR*> (TEXT(".\\MergeBoot.exe"));
	argv[1] = reinterpret_cast <_TCHAR*> (TEXT("sketch_jan09b.hex"));// hex file
	argv[2] = reinterpret_cast <_TCHAR*> (TEXT("atmega328_bootloader.txt"));// boot file
	argv[3] = reinterpret_cast <_TCHAR*> (TEXT("firmware.hex"));// new file name
	argv[4] = reinterpret_cast <_TCHAR*> (TEXT("1"));// date
	argv[5] = reinterpret_cast <_TCHAR*> (TEXT("1"));// time
#endif
	if (3 > argc || 6 < argc)
	{
		printf("Version 1.00\r\n");
		printf("Command line: <HPATH> <BPATH> [DPATH] [D] [T]\r\n");
		printf("<HPATH> : Path of Hex file\r\n");
		printf("<BPATH> : Path of boot file\r\n");
		printf("[DPATH] : File path for new name. The path do not include file extension\r\n");
		printf("[D] : Date, 0: Disable, 1: Enable\r\n");
		printf("[T] : Time, 0: Disable, 1: Enable");
		return false;
	}
	else
	{
		if (argc >= 5)
			DateFlag = atoi(argv[4]) == 1 ? 1 : 0;
		if (argc >= 6)
			TimeFlag = atoi(argv[5]) == 1 ? 1 : 0;

		// Check File Exists - fw
		Err = fopen_s(&SFile, argv[1], "rb");
		if (Err) {
			printf("can't open %s.", argv[1]);
			return false;
		}
		fclose(SFile);
		// Check File Exists - boot
		Err = fopen_s(&SFile, argv[2], "rb");
		if (Err) {
			printf("can't open %s.", argv[2]);
			return false;
		}
		fclose(SFile);
		
		//get boot line
		BootFileCount = getLineCount(argv[2]);
		//get fw line
		FirmFileCount = getLineCount(argv[1]);
		//printf("Boot line: %d, firmware line: %d\r\n", BootFileCount, FirmFileCount);

		FirmFileCount -= 1;

		GetLocalTime(&lt);
		if (argc >= 4 && argv[3] != NULL)
		{
			if (argc == 4 || (DateFlag == 1 && TimeFlag == 1))
				sprintf_s(NewFileName, MAX_PATH, "%s_%04d%02d%02d_%02d%02d.hex", argv[3], lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute);
			else if (DateFlag == 0 && TimeFlag == 0)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s.hex", argv[3]);
			}
			else if (DateFlag == 0 && TimeFlag == 1)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s_%02d%02d.hex", argv[3], lt.wHour, lt.wMinute);
			}
			else if (DateFlag == 1 && TimeFlag == 0)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s_%04d%02d%02d.hex", argv[3], lt.wYear, lt.wMonth, lt.wDay);
			}
		}
		else
		{
			sprintf_s(NewFileName, MAX_PATH, "firmware.hex");
		}
		//open or create new file
		Err = fopen_s(&DestFile, NewFileName, "w+b");
		if (Err) {
			printf("can't open %s.", NewFileName);
			return false;
		}
		//open fw file, read only
		Err = fopen_s(&SFile, argv[1], "rb");
		if (Err) {
			printf("can't open %s.", argv[1]);
			return false;
		}
		//init line
		LineCount = 1;
		//while not EOF
		while ((Char1 = getc(SFile)) != EOF)
		{
			//	get one character from fw
			if (Char1 == 0x0A)
			{
				++LineCount;
			}
			//	put one character to Dest
			putc(Char1, DestFile);
			if (LineCount >= FirmFileCount)
				break;
		}
		//close fw file
		fclose(SFile);
		if (Char1 != 0x0A && Char1 != 0x0D)
		{
			//	put CRLF characters to Dest
			putc(0x0D, DestFile);
			putc(0x0A, DestFile);
		}

		//open boot file, read only
		Err = fopen_s(&SFile, argv[2], "rb");
		if (Err) {
			printf("can't open %s.", argv[2]);
			return false;
		}
		//init line
		LineCount = 1;
		//while not EOF
		while ((Char1 = getc(SFile)) != EOF)
		{
			//	get one character from boot
			if (Char1 == 0x0A)
			{
				++LineCount;
			}
			//	put one character to Dest
			putc(Char1, DestFile);
			//if (LineCount - 1 >= BootFileCount)
			//	break;
		}
		if (Char1 != 0x0A && Char1 != 0x0D)
		{
			//	put CRLF characters to Dest
			putc(0x0D, DestFile);
			putc(0x0A, DestFile);
		}
		//close boot file
		fclose(SFile);
		//close new file
		fclose(DestFile);
		//show message
		printf("Merge to %s\r\n", NewFileName);
		// get file size
	}
	return true;
}

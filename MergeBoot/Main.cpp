// Main.cpp
//

#include <iostream>
#include <windows.h>

#define VER_STRING	"SOFTWARE_VER"
//#define SOFTWARE_VER "XXX.XXX.XX.XXX.V1.16"
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
	char *pbufHead;
	char *crlf_ind, *verstr_ind, *ver_ind, *versub_ind;
	char ln[512];
	size_t size, read_size;
	size_t len = 0;
	int llen = 0, ls = 0;
	unsigned char version_main = 0, version_sub = 0;
	int BootFileCount = 0;
	int FirmFileCount = 0;
	int DateFlag = 0;
	int TimeFlag = 0;
	int SearchFlag = 0;
	int LineCount = 1;
	int DestLineCount = 1;
	int DestFileSize = 0;
	int Char1;
	char NewFileName[MAX_PATH];
	SYSTEMTIME lt;
	errno_t Err = 0;

	ver_ind = NULL;
	versub_ind = NULL;
#if 0//test
	argc = 4;
	argv[0] = reinterpret_cast <_TCHAR*> (TEXT(".\\MergeBoot.exe"));
	argv[1] = reinterpret_cast <_TCHAR*> (TEXT("sketch_jan09b.hex"));// hex file
	argv[2] = reinterpret_cast <_TCHAR*> (TEXT("atmega328_bootloader.txt"));// boot file
	argv[3] = reinterpret_cast <_TCHAR*> (TEXT("firmware.hex"));// new file name
	argv[4] = reinterpret_cast <_TCHAR*> (TEXT("1"));// date
	argv[5] = reinterpret_cast <_TCHAR*> (TEXT("1"));// time
	argv[6] = reinterpret_cast <_TCHAR*> (TEXT("..\\Sketch.h"));// search file for get version
#endif
	if (3 > argc || 7 < argc)
	{
		printf("Version 1.01\r\n");
		printf("Command line: <HPATH> <BPATH> [DPATH] [D] [T] [SPATH]\r\n");
		printf("<HPATH> : Path of Hex file\r\n");
		printf("<BPATH> : Path of boot file\r\n");
		printf("[DPATH] : File path for new name. The path do not include file extension\r\n");
		printf("[D] : Date, 0: Disable, 1: Enable\r\n");
		printf("[T] : Time, 0: Disable, 1: Enable\r\n");
		printf("[SPATH] : File path for search version");
		return false;
	}
	else
	{
		if (argc >= 5)
			DateFlag = atoi(argv[4]) == 1 ? 1 : 0;
		if (argc >= 6)
			TimeFlag = atoi(argv[5]) == 1 ? 1 : 0;

		SearchFlag = (argc >= 7) ? 1 : 0;

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
		// Check File Exists - search
		if (SearchFlag)
		{
			Err = fopen_s(&SFile, argv[6], "rb");
			if (Err) {
				printf("can't open %s.", argv[6]);
				return false;
			}
			// get file size
			fseek(SFile, 0, SEEK_END);
			size = ftell(SFile);
			// Malloc buffer
			char* pbuf = new char[size];
			if (!pbuf)
			{
				printf("Malloc memory fail %d.", (int)size);
				fclose(SFile);
				return false;
			}
			// get data from file to buffer
			pbufHead = pbuf;
			fseek(SFile, 0, SEEK_SET);
			read_size = fread(pbuf, sizeof(unsigned char), size, SFile);
			if (read_size != size)
			{
				delete[] pbufHead;
				pbufHead = NULL;
				printf("Read file fail.");
				fclose(SFile);
				return false;
			}
			fclose(SFile);

			// search version

			ls = 0;
			//printf("\r\nFile Size : %d", size);
			while (len < size)
			{
				// get one line, limit: 512 characters
				crlf_ind = strstr(pbuf, "\r\n");
				if (!crlf_ind)
					break;//return false;
				llen = (int)((crlf_ind - pbuf) + 2);
				memset(ln, 0, 512);

				if (llen > 2)
				{
					llen -= 2;
					memcpy(ln, pbuf, llen);
				}
				else
					llen = 0;
				ln[llen] = 0;
				len += llen;
				len += 2;
				pbuf += llen;
				pbuf += 2;
				ls++;
				//printf("\r\nL[%03d,%03d,%05d]:%s", ls, llen + 2, len, l);

				verstr_ind = strstr(ln, VER_STRING);
				if (verstr_ind)
				{
					verstr_ind += sizeof(VER_STRING); // address offset
					//printf("\r\nVER_MAIN:%s", ver_ind);
					while (0x20 == *verstr_ind) verstr_ind++; // delete space character

					ver_ind = strstr(verstr_ind, ".V");
					//printf("\r\nVER_MAIN:%s", ver_ind);
					if (ver_ind)
					{
						ver_ind += 2;
						version_main = atoi(ver_ind); // get integer
						//printf("\r\nVER_MAIN:%d", version_main);

						versub_ind = strstr(ver_ind, ".");
						if (versub_ind)
						{
							versub_ind += 1;
							version_sub = atoi(versub_ind); // get integer
							//printf("\r\nVER_SUB:%d", version_sub);
						}
						else
							break;
					}
					else
						break;
				}//continue;
			}
		}
		
		//get boot line
		BootFileCount = getLineCount(argv[2]);
		//get fw line
		FirmFileCount = getLineCount(argv[1]);
		//printf("Boot line: %d, firmware line: %d\r\n", BootFileCount, FirmFileCount);

		FirmFileCount -= 1;
		memset(ln, 0, 512);
		if (SearchFlag == 1)
		{
			if (argv[3] != NULL)
			{
				if (ver_ind != NULL && versub_ind != NULL)
					sprintf_s(ln, MAX_PATH, "%s%d.%d", argv[3], version_main, version_sub);
				else if (ver_ind != NULL && versub_ind == NULL)
					sprintf_s(ln, MAX_PATH, "%s%d", argv[3], version_main);
				else if (ver_ind == NULL && versub_ind == NULL)
					sprintf_s(ln, MAX_PATH, "%s", argv[3]);

			}
			else if (argv[3] == NULL)
			{
				if (ver_ind != NULL && versub_ind != NULL)
					sprintf_s(ln, MAX_PATH, "firmware_boot_V%d.%d", version_main, version_sub);
				else if (ver_ind != NULL && versub_ind == NULL)
					sprintf_s(ln, MAX_PATH, "firmware_boot_V%d", version_main);
				else if (ver_ind == NULL && versub_ind == NULL)
					sprintf_s(ln, MAX_PATH, "firmware_boot");

			}

		}
		else
		{
			if (argv[3] != NULL)
				sprintf_s(ln, MAX_PATH, "%s", argv[3]);
			else
				sprintf_s(ln, MAX_PATH, "firmware_boot");
		}

		GetLocalTime(&lt);
		if (argc >= 4 && argv[3] != NULL)
		{
			if (argc == 4 || (DateFlag == 1 && TimeFlag == 1))
				sprintf_s(NewFileName, MAX_PATH, "%s_%04d%02d%02d_%02d%02d.hex", ln, lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute);
			else if (DateFlag == 0 && TimeFlag == 0)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s.hex", ln);
			}
			else if (DateFlag == 0 && TimeFlag == 1)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s_%02d%02d.hex", ln, lt.wHour, lt.wMinute);
			}
			else if (DateFlag == 1 && TimeFlag == 0)
			{
				sprintf_s(NewFileName, MAX_PATH, "%s_%04d%02d%02d.hex", ln, lt.wYear, lt.wMonth, lt.wDay);
			}
		}
		else
		{
			sprintf_s(NewFileName, MAX_PATH, "firmware_boot.hex");
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

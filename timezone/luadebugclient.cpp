// timezone.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "msgstruct.h"

//#include <stdio.h>
//#include <stdlib.h>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

int _tmain(int argc, _TCHAR* argv[])
{
	const char* pipename = "\\\\.\\pipe\\luadebug";


	HANDLE pipehandle;
	for(;;)
	{
		while (true)
		{
			pipehandle = CreateFile(pipename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,0,NULL);

			if (pipehandle == INVALID_HANDLE_VALUE)
			{
				DWORD dwLasError = GetLastError();
				Sleep(100);
				continue;
			}

			break;
		}


		fprintf(stdout,"connect ok\n");

		const int msgheadersize = sizeof(LPackHeader);
		const int msgcontentsize = E_NET_PACK_SIZE - msgheadersize;

		LPack pack;
		char command[64];
		BOOL bRes = TRUE;


		while (bRes)
		{			
			fprintf(stderr,"lua debug> ");
			fgets(command,sizeof(command),stdin);
			int cmd_len = strlen(command);
			if(cmd_len <= 1)
			{			
				continue;
			}

			// È¥µô '\n'
			command[--cmd_len] = '\0';

			LPackHeader* header = (LPackHeader*)&pack;
			header->size = msgheadersize + cmd_len;
			header->version = 1;
			memcpy_s((char*)(header + 1),msgcontentsize,command,cmd_len);

			DWORD dwTransBytes = 0;
			bRes = WriteFile(pipehandle,pack.buf,header->size,&dwTransBytes,NULL);
		}

		fprintf(stderr,"lua quit debug \n");
		CloseHandle(pipehandle);
	}


	return 0;
}


#define _CRT_SECURE_NO_WARNINGS
#include <conio.h>
#include <cstdlib>
#include <windows.h>

#include <iostream>
#include <string>

using namespace std;

STARTUPINFO si;
PROCESS_INFORMATION pi;

int main(int argc, char* argv[]) {

	char EventID[10];
	sprintf(EventID, " %dñ", atoi(argv[1]));
	HANDLE close = OpenEvent(EVENT_ALL_ACCESS, FALSE, EventID);
	sprintf(EventID, " %dp", atoi(argv[1]));
	HANDLE print = OpenEvent(EVENT_ALL_ACCESS, FALSE, EventID);
	char buf[256];
	sprintf(buf, " ~Compiler%d~ \0", atoi(argv[1]));

	while (true)
	{
		if (WaitForSingleObject(print, 1) == WAIT_OBJECT_0)
		{
			for (int i = 0; i < strlen(buf); i++)
			{
				if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0)
				{
					ResetEvent(print);
					CloseHandle(pi.hThread);
					CloseHandle(close);
					CloseHandle(print);
					return 0;
				}
				printf("%c", buf[i]);
				Sleep(75);
			}
			ResetEvent(print);
		}
		if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0)
		{
			CloseHandle(pi.hThread);
			CloseHandle(close);
			CloseHandle(print);
			return 0;
		}
	}

	return 0;
}

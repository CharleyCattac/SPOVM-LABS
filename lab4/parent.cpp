#define _CRT_SECURE_NO_WARNINGS
#define MAX_SECTION_SIZE 30

#include <conio.h>
#include <windows.h>
#include <iostream>

using namespace std;


PROCESS_INFORMATION initServer()
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	char args[15];
	sprintf(args, "server.exe");
	if (!CreateProcess(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Create process failed (%d)\n", GetLastError());
	}
	return pi;
}

int main()
{
	HANDLE startPrint = CreateSemaphore(NULL, 0, 1, "PRINT:START");
	HANDLE stopPrint = CreateSemaphore(NULL, 0, 1, "PRINT:STOP");
	HANDLE finPrint = CreateSemaphore(NULL, 0, 1, "PRINT:FINISH");
	HANDLE close = CreateSemaphore(NULL, 0, 1, "CLOSE");

	if (!startPrint || !stopPrint || !finPrint || !close)
	{
		cout << endl << endl << "PARENT: Errors during initialisation.";
		printf("\n");
		system("pause");
		return 0;
	}

	bool isPrinting = false;
	bool closeFlag = false;
	HANDLE section;
	LPVOID buff;
	section = CreateFileMapping(
						INVALID_HANDLE_VALUE,
						NULL,
						PAGE_READWRITE,
						0,
						MAX_SECTION_SIZE,
						"FileMap2");
	buff = MapViewOfFile(
						section,
						FILE_MAP_ALL_ACCESS,
						0,
						0,
						MAX_SECTION_SIZE);
	char uniString[MAX_SECTION_SIZE];

	PROCESS_INFORMATION server = initServer();

	cout << "Server initialised." << endl;
	cout << "You may start working." << endl;
	
	while (true)
	{
		if (WaitForSingleObject(startPrint, 1) == WAIT_OBJECT_0 && isPrinting == false)
		{
			isPrinting = true;
			memset(uniString, '\0', MAX_SECTION_SIZE);
			strncpy(uniString, (char*)buff, MAX_SECTION_SIZE);
			for (int j = 0; j < strlen(uniString); j++)
			{
				if (WaitForSingleObject(stopPrint, 1) == WAIT_OBJECT_0) {
					break;
				}
				if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0) {
					closeFlag = true;
					break;
				}
				printf("%c", uniString[j]);
				Sleep(75);
			}
			if (closeFlag)
				break;
			isPrinting = false;
			ReleaseSemaphore(finPrint, 1, NULL);
		}

		if (closeFlag) {
			break;
		}

		if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0) {
			break;
		}
	}

	WaitForSingleObject(server.hProcess, INFINITE);
	if (!CloseHandle(startPrint))
		printf("PRINT:START semaphore failed (%d)\n", GetLastError());
	if (!CloseHandle(stopPrint))
		printf("PRINT:STOP semaphore failed (%d)\n", GetLastError());
	if (!CloseHandle(finPrint))
		printf("PRINT:FINISH semaphore failed (%d)\n", GetLastError());
	if (!CloseHandle(close))
		printf("CLOSE semaphore failed (%d)\n", GetLastError());
	CloseHandle(server.hProcess);
	CloseHandle(server.hThread);
	CloseHandle(section);

	cout << endl << endl << "Server terminated.";
	printf("\n");
	system("pause");
	return 0;
}

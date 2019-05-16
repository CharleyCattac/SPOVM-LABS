#define _CRT_SECURE_NO_WARNINGS
#define MAX_SECTION_SIZE 30

#include <conio.h>
#include <cstdlib>
#include <windows.h>

#include <string>

int main(int argc, char* argv[]) {

	char procID[10];

	sprintf(procID, " %dñ", atoi(argv[1]));
	HANDLE close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, procID);

	sprintf(procID, " %df", atoi(argv[1]));
	HANDLE fileMap = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, procID);

	HANDLE print = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "Print");

	HANDLE memSection;
	memSection = OpenFileMapping(FILE_MAP_ALL_ACCESS,
								FALSE,
								"FileMap");
	LPVOID buff = MapViewOfFile(memSection,
								FILE_MAP_ALL_ACCESS,
								0,
								0,
								MAX_SECTION_SIZE);

	char empty[MAX_SECTION_SIZE];
	memset(empty, '\0', MAX_SECTION_SIZE);
	char uniStr[MAX_SECTION_SIZE];
	sprintf(uniStr, " ~Client%d~ \0", atoi(argv[1]));

	while (true)
	{
		if (WaitForSingleObject(fileMap, 1) == WAIT_OBJECT_0)
		{
			CopyMemory((PVOID)buff, empty, sizeof(empty));
			CopyMemory((PVOID)buff, uniStr, strlen(uniStr));
			ReleaseSemaphore(print, 1, NULL);
		}
		if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0)
		{
			CloseHandle(close);
			CloseHandle(fileMap);
			CloseHandle(print);
			CloseHandle(memSection);
			return 0;
		}
	}

	return 0;
}

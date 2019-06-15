#define _CRT_SECURE_NO_WARNINGS
#define MAX_AMOUNT 5
#define MAX_SECTION_SIZE 30

#include <conio.h>
#include <cstdlib>
#include <windows.h>

#include <string>
#include <vector>
#include <iostream>

using namespace std;

//mutexes for thread synchronisation
vector<HANDLE> threads;
vector<HANDLE> closeMux;
vector<HANDLE> filemapMux;
//semaphores for parent-thread synchronisation
HANDLE startPrint, stopPrint, finPrint, close;
//service fields
char procID[10];
int i = 0;
bool mappingFinished = false;

DWORD WINAPI ClientThreadRoutine(void* arg) {
	char procID[10];
	int id = threads.size();

	sprintf(procID, " %dñ", id);
	HANDLE close = OpenMutex(SYNCHRONIZE, FALSE, procID);

	sprintf(procID, " %df", id);
	HANDLE fileMap = OpenMutex(SYNCHRONIZE, FALSE, procID);

	HANDLE memSection;
	memSection = OpenFileMapping(FILE_MAP_ALL_ACCESS,
		FALSE,
		"FileMap2");
	LPVOID buff = MapViewOfFile(memSection,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		MAX_SECTION_SIZE);

	char empty[MAX_SECTION_SIZE];
	memset(empty, '\0', MAX_SECTION_SIZE);
	char uniStr[MAX_SECTION_SIZE];
	sprintf(uniStr, " ~Client%d~ \0", id);

	while (true)
	{
		if (WaitForSingleObject(fileMap, 1) == WAIT_OBJECT_0)
		{
			//cout << "WR" << id;
			CopyMemory((PVOID)buff, empty, sizeof(empty));
			CopyMemory((PVOID)buff, uniStr, strlen(uniStr));
			ReleaseMutex(fileMap);
			mappingFinished = true;
		}
		if (WaitForSingleObject(close, 1) == WAIT_OBJECT_0)
		{
			//cout << "CL" << id;
			CloseHandle(close);
			CloseHandle(fileMap);
			CloseHandle(memSection);
			return 0;
		}
	}

	return 0;
}

HANDLE createNewThread()
{
	int tmp = threads.size() + 1;
	HANDLE descr = CreateThread(
						NULL,																	 // default security attributes
						0,																		 // use default stack size  
						ClientThreadRoutine,					  							 // thread function name
						NULL,																    // argument to thread function 
						0,																	    // use default creation flags 
						NULL);																 // doesn't return the thread identifier 
	if (!descr) {
		printf("Create Handle failed (%d)\n", GetLastError());
	}
	return descr;
}

void addOneThread() {
	threads.push_back(createNewThread());

	sprintf(procID, " %dñ", threads.size());
	closeMux.push_back(CreateMutex(NULL, TRUE, procID));

	sprintf(procID, " %df", threads.size());
	filemapMux.push_back(CreateMutex(NULL, TRUE, procID));
}

void removeOneThread() {
	ReleaseMutex(closeMux.back());

	WaitForSingleObject(threads.back(), INFINITE);
	if (!CloseHandle(closeMux.back()))
		printf("Close mutex failed (%d)\n", GetLastError());
	if (!CloseHandle(filemapMux.back()))
		printf("Filemap mutex failed (%d)\n", GetLastError());
	CloseHandle(threads.back());

	closeMux.pop_back();
	filemapMux.pop_back();
	threads.pop_back();
}

int switchMenu(char key) {
	int retCode = 0;
	if (key == '+' && threads.size() < MAX_AMOUNT)
	{
		addOneThread();
		Sleep(1);
		if (threads.size() == 1) {
			ReleaseSemaphore(startPrint, 1, NULL);
		}
		return retCode;
	}
	if (key == '-' && !threads.empty())
	{
		if (i == threads.size() - 1) {
			i = -1;
			retCode = 1;
		}
		removeOneThread();
		return retCode;
	}
	if (key == 'q')
	{
		while (!threads.empty()) {
			removeOneThread();
		}
		retCode = -1;
		return retCode;
	}
	return retCode;
}

int main()
{
	Sleep(2); 
	cout << endl;
	cout << "Press.." << endl;
	cout << "\t'+' to create new thread;" << endl;
	cout << "\t'-' to delete last thread;" << endl;
	cout << "\t'q' to quit;" << endl << endl;

	startPrint = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "PRINT:START");
	stopPrint = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "PRINT:STOP");
	finPrint = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "PRINT:FINISH");
	close = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, "CLOSE");

	if (!startPrint || !stopPrint || !finPrint || !close)
	{
		cout << endl << endl << "SERVER: Errors during initialisation.";
		ReleaseSemaphore(close, 1, NULL);
		return 0;
	}

	int callbackValue;
	char key = 0;
	bool printOn = false;

	while (true)
	{
		if (!threads.empty() && WaitForSingleObject(finPrint, 1) == WAIT_OBJECT_0)
		{
			printOn = false;
			if (i == threads.size() - 1) 
				i = -1;
			i++;
			mappingFinished = false;
			ReleaseMutex(filemapMux[i]);
			while (!mappingFinished);
			WaitForSingleObject(filemapMux[i], INFINITE);
			printOn = true;
			ReleaseSemaphore(startPrint, 1, NULL);
		}

		if (_kbhit())
		{
			callbackValue = switchMenu(_getch());
			Sleep(1);
			if (callbackValue) {
				if (printOn) {
					printOn = false;
					ReleaseSemaphore(stopPrint, 1, NULL);
				}
				if (callbackValue == -1)
					break;
			}
		}
	}

	ReleaseSemaphore(close, 1, NULL);
	return 0;
}

#define _CRT_SECURE_NO_WARNINGS
#define MAX_AMOUNT_OP_PROCESSES 5

#include <conio.h>
#include <windows.h>
#include <ctime>

#include <vector>
#include <iostream>

using namespace std;

vector<PROCESS_INFORMATION> procInfo;
vector<HANDLE> closeEventInfo;
vector<HANDLE> printEventInfo;
LPDWORD exitCode;
char EventID[10];
bool canPrint = true;

PROCESS_INFORMATION createNewProcess(char * eventID)
{
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);

	PROCESS_INFORMATION pi;
	char args[30];
	sprintf(args, "child1.exe %s", eventID);
	if (!CreateProcess(NULL, args, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
		printf("Create Process failed (%d)\n", GetLastError());
	}
	return pi;
}

void addOneProcess(int id) {
	sprintf(EventID, " %d", id);
	procInfo.push_back(createNewProcess(EventID));

	sprintf(EventID, " %dñ", id);
	closeEventInfo.push_back(CreateEvent(NULL, FALSE, FALSE, EventID));

	sprintf(EventID, " %dp", id);
	printEventInfo.push_back(CreateEvent(NULL, TRUE, FALSE, EventID));
}

void removeOneProcess() {
	SetEvent(closeEventInfo.back());

	WaitForSingleObject(procInfo.back().hProcess, INFINITE);
	if (!CloseHandle(closeEventInfo.back()))
		printf("Close Handle failed (%d)\n", GetLastError());
	if (!CloseHandle(printEventInfo.back()))
		printf("Print Handle failed (%d)\n", GetLastError());
	CloseHandle(procInfo.back().hProcess);
	CloseHandle(procInfo.back().hThread);

	closeEventInfo.pop_back();
	printEventInfo.pop_back();
	procInfo.pop_back();
}

int main()
{
	cout << "Press.." << endl;
	cout << "\t'+' to create new child;" << endl;
	cout << "\t'-' to delete last child;" << endl;
	cout << "\t'q' to quit;" << endl << endl;

	//int id = 0;
	int i = 0;
	char key = 0;

	while (true)
	{
		if (!printEventInfo.empty() && WaitForSingleObject(printEventInfo[i], 1) == WAIT_TIMEOUT) {
			if (i == procInfo.size() - 1) {
				i = -1;
			}
			SetEvent(printEventInfo[++i]);
		}
		if (_kbhit())
		{
			key = _getch();
			if (key == '+' && procInfo.size() < MAX_AMOUNT_OP_PROCESSES)
			{
				addOneProcess(procInfo.size() + 1);
				continue;
			}
			if (key == '-' && !procInfo.empty())
			{
				if (i == procInfo.size() - 1) {
					i = 0;
				}
				removeOneProcess();
				if (!procInfo.empty())
					SetEvent(printEventInfo[i]);
				continue;
			}
			if (key == 'q')
			{
				while (!procInfo.empty()) {
					removeOneProcess();
				}
				printf("\n");
				system("pause");
				return 0;
			}
		}
	}

	return 0;
}

#ifdef __linux__
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <string.h>
#include <time.h>
#elif _WIN32 | _WIN64
#include <conio.h>
#include <windows.h>
#endif

#include <iostream>
using namespace std;

int main()
{
	string fileName;
	cout << "Enter name of file to be compiled:" << endl;
	cin.clear();
	rewind(stdin);
	cin >> fileName;

#ifdef __linux__
	initscr();
	curs_set(0);
#elif _WIN32 | _WIN64
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
#endif
	
#ifdef __linux__
	time_t ltime;
	int pid = fork();
	int st;

	switch (pid) {
		case -1: {
			printw("Error.");
			break;
		}

		case 0: {
			if (execl("./child", fileName.c_str(), nullptr) == -1)
				printw("Error.");
			break;
		}

		default: {
			printw("\n~~~Compiling is happening~~\n");
			while (true)
			{
				refresh();
				time(&ltime);
				move(8, 0);
				printw(ctime(&ltime));
				printw("p\n");

				if (waitpid(pid, &st, WNOHANG) == pid)
					break;
				napms(50);
			}
		}
	}

	//getch();
	endwin();
#elif _WIN32 | _WIN64
	fileName = "child.exe " + fileName;
	if (!CreateProcess(NULL,					 
		const_cast<char*>(fileName.c_str()),
		NULL,					
		NULL,					
		FALSE,				
		CREATE_NEW_CONSOLE,
		NULL,					
		NULL,					 
		&si,					
		&pi)					
		)
	{
		cout << "CreateProcess failed " << GetLastError() << endl;
		return -1;
	}

	cout << endl << "~~~Compiling is happening~~" << endl << endl;
	SYSTEMTIME lt;
	while (WaitForSingleObject(pi.hProcess, 50))
	{
		GetLocalTime(&lt);
		printf("%02d:%02d:%02d\r", lt.wHour, lt.wMinute, lt.wSecond);
	}

	CloseHandle(pi.hProcess);
#endif

	return 0;
}

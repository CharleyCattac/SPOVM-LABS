#ifdef __linux__
#include <sys/types.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

#elif _WIN32 | _WIN64
#include <conio.h>
#include <windows.h>
#endif

#include <iostream>
#include <string>
using namespace std;

void extIsAcceptable(string name) {
#ifdef _WIN32 | _WIN64
	cout << "File " << name << ".exe is ready!" << endl << endl;
#elif __linux__
	printw("File %s.exe is ready!\n\n", name.c_str());
#endif
}

void extIsDangerous(string name) {
#ifdef _WIN32 | _WIN64
	cout << "File " << name << ".exe is ready!" << endl;
	cout << "WARNING! Direct executing may be unsafe!" << endl << endl;
#elif __linux__
	printw("File %s.exe is ready!\n", name.c_str());
	printw("WARNING! Direct executing may be unsafe!\n\n");
#endif
}

void extIsNotAcceptable(string fileName) {
#ifdef _WIN32 | _WIN64
	cout << "File " << fileName << " cannot be compiled!" << endl;
	cout << "Reason:\tExtension is unacceptable or absent." << endl;
#elif __linux__
	printw("File %s cannot be compiled!\n", fileName.c_str());
	printw("Reason:\tExtension is unacceptable or absent.\n\n");
#endif
}


int main(int argc, char* argv[]) {
	string acceptableExts[] = { "txt" , "cpp", "c", "java" };
	string dangerousExts[] = { "js", "py" };

#ifdef _WIN32 | _WIN64
	string fileName(argv[1]);
#elif __linux__
	initscr();
	noecho();
	string fileName(argv[0]);
#endif

	string name, ext;
	bool outputIsPerformed = false;

	if (fileName.find_last_of(".") != string::npos) {
      name = fileName.substr(0, fileName.find_last_of("."));
      ext = fileName.substr(fileName.find_last_of(".") + 1, fileName.size() - 1);

      for (int i = 0; i < 3; i++)
         if (ext == acceptableExts[i]) {
            extIsAcceptable(name);
            outputIsPerformed = true;
         }

      if (!outputIsPerformed)
         for (int i = 0; i < 2; i++)
            if (ext == dangerousExts[i]) {
               extIsDangerous(name);
               outputIsPerformed = true;
            }

      if (!outputIsPerformed) {
         extIsNotAcceptable(fileName);
      }
   }

	else {
		extIsNotAcceptable(fileName);
	}

#ifdef _WIN32 | _WIN64
	SYSTEMTIME lt;
	
	while (kbhit() == 0) {
		GetLocalTime(&lt);
		printf("%02d:%02d:%02d\r", lt.wHour, lt.wMinute, lt.wSecond);
		Sleep(50);
	}
#elif __linux__
	char c = '0';
	time_t ltime;

	while (c != 27) {
		time(&ltime);
		move(10, 0);
		printw(ctime(&ltime));
		printw("c\n");

		halfdelay(2);
		c = getch();
		refresh();
		move(14, 0);
		printw("Press ESC to quit\n");
	}

#endif

	return 0;
}

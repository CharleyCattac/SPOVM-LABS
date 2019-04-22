#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <ncurses.h>

#include <stdlib.h>
#include <iostream>
#include <string.h>

using namespace std;

struct sigaction printSignal, closeSignal;
bool Print = false;
bool Close = false;
char parentPID[256];
/*
 * SIGNAL          PURPOSE
 * ****************************************************
 * THE FOLLOWING SIGNALS COULD BE RECEIVED FROM PARENT:
 * SIGUSR1         start printing
 * SGNUSR2         close the process
 * ****************************************************
 * THE FOLLOWING SIGNALS COULD BE SENT TO PARENT:
 * SIGUSR1         allow sending print signals
 * ****************************************************
 */

void setPrint(int sign) {
   Print = true;
}

void setClose(int sign) {
   Close = true;
}

void initSignalHandlers() {
   printSignal.sa_handler = setPrint;
   sigaction(SIGUSR1, &printSignal, NULL);

   closeSignal.sa_handler = setClose;
   sigaction(SIGUSR2, &closeSignal, NULL);
}

/*
 * ARGUMENT          MEANING
 * ****************************************************
 * ADGV[0]           ID
 * ADGV[1]           parent's PID
 * ****************************************************
 *
 */
int main(int argc, char* argv[]) {

   sprintf(parentPID, "%d", atoi(argv[1]));
   initSignalHandlers();

	char buf[256];
   sprintf(buf, " ~Compiler%d~ ", atoi(argv[0]));

	while (true) {
	   usleep(10000);
	   if (Print) {
         Print = false;
			for (int i = 0; i < strlen(buf); i++) {
				if (Close) {
               endwin();
               return 0;
				}
				cout << buf[i] << endl;
				napms(150);
			}
			kill(atoi(parentPID), SIGUSR1);
		}
		if (Close) {
         break;
		}
	}

	endwin();
	return 0;
}

#define MAX_AMOUNT_OP_PROCESSES 5

#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <termios.h>
#include <ncurses.h>
//#include <sys/sysinfo.h>

#include <vector>
#include <iostream>

using namespace std;

vector<pid_t> procInfo;
struct sigaction printSignal; //from child1
bool Print = true;
char parentPID[256];
int i = 0;

int myGetch() {
   struct termios oldattr, newattr;
   int ch;
   tcgetattr(STDIN_FILENO, &oldattr);
   newattr = oldattr;
   newattr.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
   ch = getchar();
   tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
   return ch;
}
/*
 * SIGNAL          PURPOSE
 * **********************************************
 * THE FOLLOWING SIGNALS COULD BE SENT TO CHILD1:
 * SIGUSR1         start printing
 * SGNUSR2         close the process
 * **********************************************
 * THE FOLLOWING SIGNALS COULD BE RECEIVED FROM CHILD1:
 * SIGUSR1         finished printing
 * **********************************************
 */

void setPrint(int sig) {
   if (procInfo.size()) {
      if (++i >= procInfo.size()) {
         i = 0;
      }
      napms(150);
      //sleep(1);
      kill(procInfo[i], SIGUSR1);

   } else {
      Print = true;
   }
}

void initSignalHandlers() {
   printSignal.sa_handler = setPrint;
   printSignal.sa_flags =  SA_RESTART;
   sigaction(SIGUSR1, &printSignal, NULL);
}

void addOneProcess() {
   procInfo.push_back(fork());

   if (procInfo.back() == 0) {
      char instanceID[10];
      sprintf(instanceID, "%d", procInfo.size());
      if (execlp("./child1", instanceID, parentPID, NULL) == -1) {
         procInfo.pop_back();
         cout << "Error." << endl;
      }
   }
}

void removeOneProcess() {
   kill(procInfo.back(), SIGUSR2);
   waitpid(procInfo.back(), NULL, 0);

   procInfo.pop_back();
}

void showOptionList() {
   cout << "Press..\n";
   cout << "   '+' to create new child;" << endl;
   cout << "   '-' to delete last child;" << endl;
   cout << "   'q' to quit;\n" << endl;
}

int main()
{

   showOptionList();

   initSignalHandlers();
   sprintf(parentPID, "%d", getpid());


	while (true) {
      switch(myGetch()) {
         case '+': {
            if(procInfo.size() < MAX_AMOUNT_OP_PROCESSES){
               addOneProcess();
               if (Print == true) {
                  napms(150);
                  //sleep(1);
                  kill(procInfo.back(), SIGUSR1);
                  Print = false;
               }
            }
            break;
         }
         case '-': {
            if (!procInfo.empty()) {
               if (i == procInfo.size() - 1) {
                  removeOneProcess();
                  raise(SIGUSR1);
               } else {
                  removeOneProcess();
               }
            }
            break;
         }
         case 'q': {
            while (!procInfo.empty()) {
               removeOneProcess();
            }
            cout << "\nPress any key to proceed...\n";
            myGetch();
            return 0;
         }
      }
	}

	return 0;
}

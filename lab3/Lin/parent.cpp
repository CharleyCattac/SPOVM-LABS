#define _CRT_SECURE_NO_WARNINGS
#define MAX_AMOUNT_OP_PROCESSES 5
#define MAX_MEM_SIZE 30

#include <stdlib.h>
#include <sys/sem.h>
#include <wait.h>

#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <vector>
#include <iostream>
#include <string.h>

using namespace std;
/*
 * NUMBER   SEMAPHOS
 * 0        read and print (server)
 * 1-5      write (client)
 * 6-10     close (client)
 */
vector<pid_t> processes;
char procID[10];
char pipeWstr[10];
char semID[10];
int sem_id;
int pipeW;
int pipeR;
int i = 0;

union semun
{
    int              val;
    struct semid_ds *buf;
    unsigned short  *array;
    struct seminfo  *__buf;
} semunion;

int WaitSemaphore(int sem_id, int num)
{
   struct sembuf buf;
   buf.sem_op = -1;
   buf.sem_flg = IPC_NOWAIT;
   buf.sem_num = num;
   return semop(sem_id, &buf, 1);
}

int ReleaseSemaphore(int sem_id, int num)
{
   struct sembuf buf;
   buf.sem_op = 1;
   buf.sem_flg = IPC_NOWAIT;
   buf.sem_num = num;
   return semop(sem_id, &buf, 1);
}

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

bool kbhit()
{
   termios term;
   tcgetattr(0, &term);

   termios term2 = term;
   term2.c_lflag &= ~ICANON;
   tcsetattr(0, TCSANOW, &term2);

   int byteswaiting;
   ioctl(0, FIONREAD, &byteswaiting);

   tcsetattr(0, TCSANOW, &term);

   return byteswaiting > 0;
}

void createNewProcess()
{
	processes.push_back(fork());

	if (processes.back() == 0) {
		char instanceID[10];
		sprintf(instanceID, "%d", processes.size());
		if (execlp("./child1", instanceID, pipeWstr, semID, NULL) == -1) {
			processes.pop_back();
			cout << "Error.\n";
		}
	}
}

void addOneProcess(int id) {
	createNewProcess();
}

void removeOneProcess() {
   ReleaseSemaphore(sem_id, MAX_AMOUNT_OP_PROCESSES + processes.size());

	waitpid(processes.back(), NULL, 0);
	processes.pop_back();
}

int switchMenu(char key) {
	int retCode = 0;
	if (key == '+' && processes.size() < MAX_AMOUNT_OP_PROCESSES)
	{
		addOneProcess(processes.size() + 1);
      usleep(5000);
		if (processes.size() == 1) {
         ReleaseSemaphore(sem_id, i + 1);
		}
		return retCode;
	}
	if (key == '-' && !processes.empty())
	{
		if (i == processes.size() - 1) {
			i = -1;
			retCode = 1;
		}
		removeOneProcess();
		return retCode;
	}
	if (key == 'q')
	{
		while (!processes.empty()) {
			removeOneProcess();
		}
		retCode = -1;
		return retCode;
	}
	return retCode;
}

int main()
{
   cout << "Press..\n";
   cout << "\t'+' to create new child;\n";
   cout << "\t'-' to delete last child;\n";
	cout << "\t'q' to quit;\n\n";

   sem_id = semget(IPC_PRIVATE, MAX_AMOUNT_OP_PROCESSES * 2 + 1, IPC_CREAT|0777);
   semctl(sem_id, 0, SETALL, 0);
   if(sem_id == -1) return 0;
   sprintf(semID, "%d", sem_id);

   bool isPrinting = false;
	char uniString[MAX_MEM_SIZE];

	int pipeID[2];
	if(pipe(pipeID)){
      semctl(sem_id, 0, IPC_RMID, semunion);
      cout << "\nPress any key to proceed...\n";
      myGetch();
      return 0;
	}
	pipeR = pipeID[0];
   pipeW = pipeID[1];
   sprintf(pipeWstr, "%d", pipeID[1]);

	int switchCallback;
	int key = 0;

	while (true)
	{
		if (!processes.empty() && !WaitSemaphore(sem_id, 0) && isPrinting == false)
		{
			isPrinting = true;

         memset(uniString, '\0', MAX_MEM_SIZE);
			read(pipeR, &uniString, MAX_MEM_SIZE);
			for (int j = 0; j < strlen(uniString); j++)
			{
            if (kbhit()) {
               key = myGetch();
               if (key != -1) {
                  switchCallback = switchMenu((char) key);
						usleep(1000);
                  if (switchCallback)
                     break;
               }
            }
 				cout << uniString[j] << endl;
				usleep(100000);
			}
			if (switchCallback == -1)
				break;
			if (i == processes.size() - 1) {
				i = -1;
			}
			isPrinting = false;
			ReleaseSemaphore(sem_id, ++i + 1);
		}

      if (kbhit()) {
         key = myGetch();
         if (key != -1) {
            switchCallback = switchMenu((char) key);
            if (switchCallback == -1)
               break;
         }
      }
	}

   close(pipeW);
	close(pipeR);
   semctl(sem_id, 0, IPC_RMID, semunion);
   cout << "\nPress any key to proceed..." << endl;
   myGetch();
	return 0;
}

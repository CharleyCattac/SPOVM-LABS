#define _CRT_SECURE_NO_WARNINGS
#define MAX_MEM_SIZE 30

#include <stdlib.h>
#include <sys/sem.h>
#include <wait.h>

#include <unistd.h>
#include <sys/wait.h>
#include <termios.h>

#include <vector>
#include <iostream>
#include <string.h>

using namespace std;

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

int main(int argc, char* argv[]) {

	int id = atoi(argv[0]);
	int pipeWR = atoi(argv[1]);
	char uniStr[MAX_MEM_SIZE];
   memset(uniStr, '\0', MAX_MEM_SIZE);
	sprintf(uniStr, " ~Client%d~ \0", id);

   int sem_id = atoi(argv[2]);

	while (true)
	{
      usleep(10000);
		if (!WaitSemaphore(sem_id, id))
		{
			write(pipeWR, uniStr, sizeof(uniStr));
         ReleaseSemaphore(sem_id, 0);
		}
		if (!WaitSemaphore(sem_id, id + 5))
		{
         break;
		}
	}

	return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main()
{
   int shmid, num, *buf;
   int size = 10;
   char c;

   shmid = shmget(2461, sizeof(int) * (size + 4), IPC_CREAT | 00600);
   if (shmid == -1)
   {
      perror("shmget");
      return 1;
   }

   buf = (int *)shmat(shmid, NULL, 0);
   if (buf == (int *)-1)
   {
      perror("shmat");
      return 1;
   }

   // Initialize on first run
   if (buf[12] != 1 && buf[13] != 1)
   {
      buf[10] = 0;
      buf[11] = 0;
   }

   buf[12] = 1;  // Producer is running

   while (1)
   {
      // Wait for buffer space (circular queue not full)
      while (((buf[10] + 1) % size) == buf[11]);

      printf("Enter Data: ");
      scanf("%d", &num);
      getchar();

      buf[buf[10]] = num;
      buf[10] = (buf[10] + 1) % size;

      printf("Do you want to continue (y/n): ");
      scanf("%c", &c);
      getchar();

      if (c != 'y' && c != 'Y')
      {
         break;
      }
   }

   buf[12] = 0;  // Producer stopped

   if (buf[13] == 0)
   {
      shmdt(buf);
      shmctl(shmid, IPC_RMID, NULL);
   }
   else
   {
      shmdt(buf);
   }

   return 0;
}
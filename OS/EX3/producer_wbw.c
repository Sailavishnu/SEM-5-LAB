#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <string.h>
#include <unistd.h>

int main()
{
   int shmid, num, *buf;
   int size = 10;

   // Create shared memory
   shmid = shmget(2461, sizeof(int) * 20, IPC_CREAT | IPC_EXCL | 0666);
   if (shmid == -1)
   {
      shmid = shmget(2461, sizeof(int) * 20, 0666);
      shmctl(shmid, IPC_RMID, NULL);
      shmid = shmget(2461, sizeof(int) * 20, IPC_CREAT | 0666);
   }

   buf = (int *)shmat(shmid, NULL, 0);
   memset(buf, 0, sizeof(int) * 20);

   buf[10] = 0;   // write ptr
   buf[11] = 0;   // read ptr
   buf[12] = 1;   // producer running
   buf[14] = 0;   // production count
   buf[15] = 0;   // consumption count

   printf("=== PRODUCER STARTED ===\n");
   printf("Enter values (enter -1 to exit):\n\n");

   while (1)
   {
      // Case 1: Buffer full - wait
      while (((buf[10] + 1) % size) == buf[11])
      {
         printf("[PRODUCER BLOCKED - Buffer FULL]\n");
      }

      printf("Enter: ");
      fflush(stdout);
      scanf("%d", &num);
      getchar();

      if (num == -1)
         break;

      // Case 3: Write to buffer
      buf[buf[10]] = num;
      buf[10] = (buf[10] + 1) % size;
      buf[14]++;  // SIGNAL: increment production count

      printf("  ✓ Produced: %d (count=%d)\n", num, buf[14]);
   }

   buf[12] = 0;  // Producer stopped
   printf("\n[PRODUCER STOPPED]\n");
   sleep(1);

   shmdt(buf);
   return 0;
}
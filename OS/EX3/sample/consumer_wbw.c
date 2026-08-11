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

   printf("=== CONSUMER STARTED ===\n");
   printf("Waiting for producer to create shared memory...\n");

   // Attach to shared memory (size = 0 for existing segment)
   while ((shmid = shmget(2461, 0, 0666)) == -1)
   {
      printf(".");
      fflush(stdout);
      sleep(1);
   }

   printf("\nShared memory attached!\n\n");

   buf = (int *)shmat(shmid, NULL, 0);

   printf("Waiting for data...\n\n");

   while (1)
   {
      // Case 2: Wait for data (production count > consumption count)
      while (buf[14] == buf[15])
      {
         if (buf[12] == 0)  // Producer stopped AND no more data
         {
            printf("\n[CONSUMER DONE - Producer finished]\n");
            goto cleanup;
         }
         // Otherwise just wait, don't print spam
      }

      // Case 3: Consume data
      num = buf[buf[11]];
      buf[11] = (buf[11] + 1) % size;
      buf[15]++;  // Signal: increment consumption count

      printf("  ← Consumed: %d (count=%d)\n", num, buf[15]);
   }

cleanup:
   shmdt(buf);
   shmctl(shmid, IPC_RMID, NULL);
   return 0;
}
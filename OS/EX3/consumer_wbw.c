#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

#define SIZE 10
#define KEY 2461
#define SHM_SIZE (sizeof(int) * 20)

int main()
{
    int shmid, num, *buf;

    printf("Consumer started. Waiting for producer...\n");

    // Wait until producer creates shared memory
    while ((shmid = shmget(KEY, SHM_SIZE, 0666)) == -1) {
        sleep(1);
    }

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1) {
        perror("shmat");
        return 1;
    }

    // Wait until producer finishes initialization
    while (buf[13] != 1)
        sleep(1);

    printf("Shared memory ready. Consuming data...\n");

    while (1) {
        // Busy wait while buffer is empty
        while (buf[10] == buf[11]) {
            if (buf[12] == 0) {
                printf("Producer finished. Consumer stopping.\n");

                shmdt(buf);
                shmctl(shmid, IPC_RMID, NULL);
                return 0;
            }
        }

        num = buf[buf[11]];
        buf[11] = (buf[11] + 1) % SIZE;

        printf("Consumed: %d\n", num);
    }
}
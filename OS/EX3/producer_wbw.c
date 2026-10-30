#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define SIZE 10
#define KEY 2461
#define SHM_SIZE (sizeof(int) * 20)

int main()
{
    int shmid, num, *buf;

    // Create a NEW shared memory segment
    shmid = shmget(KEY, SHM_SIZE, IPC_CREAT | IPC_EXCL | 0666);

    if (shmid == -1) {
        printf("Shared memory already exists or creation failed.\n");
        printf("Check ipcs -m and remove any stale segment first.\n");
        return 1;
    }

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1) {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return 1;
    }

    // Initialize shared variables
    buf[10] = 0;  // Write pointer
    buf[11] = 0;  // Read pointer
    buf[12] = 1;  // Producer running
    buf[13] = 1;  // Initialization complete

    printf("Producer started. Enter -1 to stop.\n");

    while (1) {
        // Busy wait while buffer is full
        while ((buf[10] + 1) % SIZE == buf[11])
            ;

        printf("Enter data: ");

        if (scanf("%d", &num) != 1) {
            printf("Invalid input.\n");
            break;
        }

        if (num == -1)
            break;

        buf[buf[10]] = num;
        buf[10] = (buf[10] + 1) % SIZE;

        printf("Produced: %d\n", num);
    }

    buf[12] = 0;  // Producer finished

    printf("Producer finished.\n");

    shmdt(buf);
    return 0;
}
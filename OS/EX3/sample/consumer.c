#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 10

int main()
{
    int shmid;
    int *buf;

    // Access existing shared memory
    shmid = shmget(4777, sizeof(int) * (SIZE + 2), 0666);

    if (shmid == -1)
    {
        perror("Error");
        printf("\nShared memory not found.\n");
        printf("Please execute the Producer program first.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory located successfully.\n");

    // Attach shared memory
    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("Error");
        printf("\nFailed to attach shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory attached successfully.\n");

    printf("\n========== CONSUMER ==========\n");

    for (int i = 0; i < SIZE; i++)
    {
        printf("Data[%d] = %d\n", i + 1, buf[buf[11]]);
        buf[11] = (buf[11] + 1) % SIZE;
    }

    if (shmdt(buf) == -1)
    {
        perror("Error");
        printf("Failed to detach shared memory.\n");
    }
    else
    {
        printf("\nConsumer detached successfully.\n");
    }

    if (shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("Error");
        printf("Failed to delete shared memory.\n");
    }
    else
    {
        printf("Shared memory deleted successfully.\n");
    }

    return 0;
}
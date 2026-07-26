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

    // Create shared memory
    shmid = shmget(4777, sizeof(int) * (SIZE + 2), IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("Error");
        printf("\nFailed to create shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory created successfully.\n");

    // Attach shared memory
    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("Error");
        printf("\nFailed to attach shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory attached successfully.\n");

    // Initialize
    buf[10] = 0;   // in
    buf[11] = 0;   // out

    printf("\n========== PRODUCER ==========\n");

    for (int i = 0; i < SIZE; i++)
    {
        printf("Enter data %d : ", i + 1);
        scanf("%d", &buf[buf[10]]);

        buf[10] = (buf[10] + 1) % SIZE;
    }

    printf("\nAll data written successfully.\n");

    if (shmdt(buf) == -1)
    {
        perror("Error");
        printf("Failed to detach shared memory.\n");
    }
    else
    {
        printf("Producer detached from shared memory successfully.\n");
    }

    return 0;
}
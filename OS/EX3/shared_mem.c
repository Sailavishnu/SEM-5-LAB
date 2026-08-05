#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int shmid, *buf;
    int i, num, size;

    printf("Enter the size of the shared memory: ");
    if (scanf("%d", &size) != 1 || size <= 0) 
    {
        printf("Invalid size entered.\n");
        exit(EXIT_FAILURE);
    }

    shmid = shmget(4777, sizeof(int) * size, IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("Error");
        printf("Failed to create shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory created successfully.\n");

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("Error");
        printf("Failed to attach shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory attached successfully.\n");

    printf("\n========== PRODUCER ==========\n");

    for (i = 0; i < size; i++)
    {
        printf("Enter data %d : ", i + 1);
        scanf("%d", &num);
        buf[i] = num;
    }

    printf("\nAll data written successfully.\n");

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Error");
        printf("Failed to create child process.\n");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        printf("\n========== CONSUMER ==========\n");

        for (i = 0; i < size; i++)
        {
            printf("Data[%d] = %d\n", i + 1, buf[i]);
        }

        if (shmdt(buf) == -1)
        {
            perror("Error");
            printf("Consumer failed to detach shared memory.\n");
        }
        else
        {
            printf("\nConsumer detached from shared memory successfully.\n");
        }

        exit(0);
    }
    else
    {
        wait(NULL);

        if (shmdt(buf) == -1)
        {
            perror("Error");
            printf("Producer failed to detach shared memory.\n");
        }
        else
        {
            printf("Producer detached from shared memory successfully.\n");
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
    }

    return 0;
}

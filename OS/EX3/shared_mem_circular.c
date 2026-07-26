#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SIZE 10

struct shared
{
    int buffer[SIZE];
    int in;
    int out;
    int count;
};

int main()
{
    int shmid;
    struct shared *shm;

    // Create shared memory
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("Error");
        printf("Failed to create shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory created successfully.\n");

    // Attach shared memory
    shm = (struct shared *)shmat(shmid, NULL, 0);

    if (shm == (void *)-1)
    {
        perror("Error");
        printf("Failed to attach shared memory.\n");
        exit(EXIT_FAILURE);
    }

    printf("Shared memory attached successfully.\n");

    // Initialize
    shm->in = 0;
    shm->out = 0;
    shm->count = 0;

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Error");
        printf("Failed to create child process.\n");
        exit(EXIT_FAILURE);
    }

    else if (pid == 0)
    {
        // Producer (Child)
        int input[SIZE] = {10,20,30,40,50,60,70,80,90,100};

        printf("\n========== PRODUCER ==========\n");

        for (int i = 0; i < SIZE; i++)
        {
            if (shm->count == SIZE)
            {
                printf("Buffer is Full.\n");
                break;
            }

            shm->buffer[shm->in] = input[i];

            printf("Produced %d at index %d\n",
                   input[i], shm->in);

            shm->in = (shm->in + 1) % SIZE;
            shm->count++;
        }

        if (shmdt(shm) == -1)
        {
            perror("Error");
            printf("Producer failed to detach shared memory.\n");
        }
        else
        {
            printf("\nProducer detached from shared memory successfully.\n");
        }

        exit(0);
    }

    else
    {
        // Parent waits for producer
        wait(NULL);

        printf("\n========== CONSUMER ==========\n");

        while (shm->count > 0)
        {
            printf("Consumed %d from index %d\n",
                   shm->buffer[shm->out],
                   shm->out);

            shm->out = (shm->out + 1) % SIZE;
            shm->count--;
        }

        if (shmdt(shm) == -1)
        {
            perror("Error");
            printf("Consumer failed to detach shared memory.\n");
        }
        else
        {
            printf("\nConsumer detached from shared memory successfully.\n");
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
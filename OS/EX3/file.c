1
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int sid, d, in, out, num, *buf, si, i;
    int size = 10;
    in = 0;
    out = 0;

    si = shmget(4777, sizeof(int) * size, IPC_CREAT | 00600);

    if (si == -1)
    {
        perror("shmget");
        exit(1);
    }

    buf = (int *) shmat(si, NULL, 0);

    if (buf == (int *) -1)
    {
        perror("shmat");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        exit(1);
    }
    else if (pid > 0)
    {
        /* Parent: Producer */
        for (i = 0; i < size; i++)
        {
            printf("Enter the data: \n");
            scanf("%d", &num);

            buf[in % size] = num;
            in++;
        }

        shmdt(buf);
        wait(NULL);                       /* wait for child to finish reading */
        shmctl(si, IPC_RMID, NULL);       /* cleanup after child is done */
    }
    else
    {
        /* Child: Consumer */
        printf("Data Read\n");
        for (i = 0; i < size; i++)
        {
            num = buf[out % size];
            out++;
            printf("Data[%d]: %d\n", out, num);
        }

        shmdt(buf);
    }

    return 0;
}

2
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main()
{
    int shmid, *buf;
    int i, num;
    int SIZE = 10;

    shmid = shmget(4777, sizeof(int) * (SIZE + 2), IPC_CREAT | 0666);

    if (shmid == -1)
    {
        perror("shmget");
        return 0;
    }

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("shmat");
        return 0;
    }

    /* Store in and out at the end */
    buf[10] = 0;   // in
    buf[11] = 0;   // out

    for (i = 0; i < SIZE; i++)
    {
        printf("Enter data: ");
        scanf("%d", &num);

        buf[buf[10]] = num;          // Write data
        buf[10] = (buf[10] + 1) % SIZE; // Update in
    }

    shmdt(buf);

    return 0;
}


#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>

int main()
{
    int shmid, *buf;
    int i, num;
    int SIZE = 10;

    shmid = shmget(4777, sizeof(int) * (SIZE + 2), 0666);

    if (shmid == -1)
    {
        perror("shmget");
        return 0;
    }

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("shmat");
        return 0;
    }

    printf("Data Read:\n");

    for (i = 0; i < SIZE; i++)
    {
        num = buf[buf[11]];   // Read using out
        printf("%d\n", num);

        buf[11] = (buf[11] + 1) % SIZE; // Update out
    }

    shmdt(buf);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}

3
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define SIZE 10

struct shared {
    int buffer[SIZE];
    int in;
    int out;
    int count;
};

int main() {
    int shmid;
    struct shared *shm;

    // Create shared memory before fork
    shmid = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | 0666);

    if (shmid < 0) {
        printf("Shared memory creation failed\n");
        exit(1);
    }

    shm = (struct shared *)shmat(shmid, NULL, 0);

    shm->in = 0;
    shm->out = 0;
    shm->count = 0;

    pid_t pid = fork();

    if (pid == 0) {
        // Child Process - Producer
        int input[SIZE] = {10,20,30,40,50,60,70,80,90,100};

        printf("Producer (Child):\n");

        for (int i = 0; i < SIZE; i++) {

            if (shm->count == SIZE) {
                printf("Buffer Full\n");
                break;
            }

            shm->buffer[shm->in] = input[i];

            printf("Produced %d at index %d\n",
                   input[i], shm->in);

            shm->in = (shm->in + 1) % SIZE;
            shm->count++;
        }

        shmdt(shm);
        exit(0);
    }
    else {
        // Parent Process - Consumer
        wait(NULL);

        printf("\nConsumer (Parent):\n");

        while (shm->count > 0) {

            printf("Consumed %d from index %d\n",
                   shm->buffer[shm->out],
                   shm->out);

            shm->out = (shm->out + 1) % SIZE;
            shm->count--;
        }

        shmdt(shm);

        // Delete shared memory
        shmctl(shmid, IPC_RMID, NULL);
    }

    return 0;
}
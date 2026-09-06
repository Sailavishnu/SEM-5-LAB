#include "semaphore.h"

int main()
{
    int shmid, sid, *buf;
    int num;
    char choice = 'y';

    printf("=== CONSUMER STARTED ===\n\n");

    /* Access shared memory created by Producer */
    shmid = shmget(
        SHM_KEY,
        sizeof(int) * 12,
        0666
    );

    if (shmid == -1)
    {
        perror("shmget");

        printf("\nStart Producer first!\n");

        exit(1);
    }

    /* Attach shared memory */
    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    /* Access semaphore set */
    sid = semget(
        SEM_KEY,
        3,
        0666
    );

    if (sid == -1)
    {
        perror("semget");
        exit(1);
    }

    printf("Waiting for data...\n\n");

    while (choice == 'y' || choice == 'Y')
    {
        /*
           WAIT(full)

           If there is no data,
           Consumer BLOCKS here.
        */
        wait_sem(sid, full);

        /* Enter critical section */
        wait_sem(sid, mutex);

        /* Read data */
        num = buf[buf[READ_PTR]];

        printf("Consumed Data: %d\n", num);

        /* Move read pointer */
        buf[READ_PTR] =
            (buf[READ_PTR] + 1) % SIZE;

        /* Leave critical section */
        signal_sem(sid, mutex);

        /* SIGNAL(empty) */
        signal_sem(sid, empty);

        printf("Do you want to continue (y/n): ");
        scanf(" %c", &choice);

        printf("\n");
    }

    printf("[CONSUMER STOPPED]\n");

    shmdt(buf);

    return 0;
}
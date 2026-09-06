#include "semaphore.h"
#include <string.h>

int main()
{
    int shmid, sid, *buf;
    int num;
    int created = 0;
    char choice = 'y';

    union semaphore snum;
    unsigned short a[3];

    printf("=== PRODUCER STARTED ===\n\n");

    /* Try to create shared memory */
    shmid = shmget(
        SHM_KEY,
        sizeof(int) * 12,
        IPC_CREAT | IPC_EXCL | 0666
    );

    /* If already exists, attach to it */
    if (shmid == -1)
    {
        shmid = shmget(
            SHM_KEY,
            sizeof(int) * 12,
            0666
        );
    }
    else
    {
        created = 1;
    }

    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }

    /* Attach shared memory */
    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }

    /* Try to create semaphore set */
    sid = semget(
        SEM_KEY,
        3,
        IPC_CREAT | IPC_EXCL | 0666
    );

    /* If already exists, access it */
    if (sid == -1)
    {
        sid = semget(
            SEM_KEY,
            3,
            0666
        );
    }

    if (sid == -1)
    {
        perror("semget");
        exit(1);
    }

    /*
       Initialize only when Producer
       creates shared memory for first time
    */
    if (created == 1)
    {
        memset(buf, 0, sizeof(int) * 12);

        /* Shared circular buffer pointers */
        buf[WRITE_PTR] = 0;
        buf[READ_PTR] = 0;

        /* Initialize semaphores */
        a[mutex] = 1;
        a[full] = 0;
        a[empty] = SIZE;

        snum.array = a;

        semctl(sid, 0, SETALL, snum);
    }

    while (choice == 'y' || choice == 'Y')
    {
        /*
           WAIT(empty)

           If buffer is full,
           Producer automatically blocks here.
        */
        wait_sem(sid, empty);

        printf("Enter Data: ");
        fflush(stdout);

        scanf("%d", &num);

        /* Enter critical section */
        wait_sem(sid, mutex);

        /* Store data */
        buf[buf[WRITE_PTR]] = num;

        printf("Produced Data: %d\n", num);

        /* Move write pointer */
        buf[WRITE_PTR] =
            (buf[WRITE_PTR] + 1) % SIZE;

        /* Leave critical section */
        signal_sem(sid, mutex);

        /* SIGNAL(full) */
        signal_sem(sid, full);

        printf("Do you want to continue (y/n): ");
        scanf(" %c", &choice);

        printf("\n");
    }

    printf("[PRODUCER STOPPED]\n");

    shmdt(buf);

    return 0;
}
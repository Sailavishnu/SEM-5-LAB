#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
int main()
{
    int shmid, num, *buf, i, c;
    int size = 10;

    shmid = shmget(2461, sizeof(int) * (size + 4), IPC_CREAT | 00600);
    if (shmid == -1)
    {
        perror("shmget");
        return 1;
    }

    buf = (int *)shmat(shmid, NULL, 0);
    if (buf == (int *)-1)
    {
        perror("shmat");
        return 1;
    }
    if (buf[12] != 1)
    {
        buf[10] = 0;
        buf[11] = 0;
    }

    buf[13] = 1;

    while (1)
    {
        while (buf[11] == buf[10])
            ;

        num = buf[buf[11]];
        printf("Consumed Data: %d\n", num);

        buf[11] = (buf[11] + 1) % size;

        printf("Do you want to continue (y/n): ");
        scanf(" %c", &c);
        if (c != 'y' && c != 'Y')
        {
            break;
        }
    }

    buf[13] = 0;

    if (buf[12] == 0)
    {
        shmdt(buf);
        shmctl(shmid, IPC_RMID, NULL);
    }
    else
    {
        shmdt(buf);
    }

    return 0;
}
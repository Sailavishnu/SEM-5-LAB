
#include "semaphore.h"

int main() {
    int shmid, sid, *buf;
    int num, created = 0, sem_created = 0;
    char choice = 'y';

    union semaphore snum;
    unsigned short a[3];

    printf("=== PRODUCER STARTED ===\n\n");

    shmid = shmget(SHM_KEY, sizeof(int) * 12,
                   IPC_CREAT | IPC_EXCL | 0666);

    if (shmid == -1) {
        shmid = shmget(SHM_KEY, sizeof(int) * 12, 0666);
    } else {
        created = 1;
    }

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1) {
        perror("shmat");
        return 1;
    }

    sid = semget(SEM_KEY, 3,
                 IPC_CREAT | IPC_EXCL | 0666);

    if (sid == -1) {
        sid = semget(SEM_KEY, 3, 0666);
    } else {
        sem_created = 1;
    }

    if (sid == -1) {
        perror("semget");
        shmdt(buf);
        return 1;
    }

    if (created) {
        memset(buf, 0, sizeof(int) * 12);
        buf[WRITE_PTR] = 0;
        buf[READ_PTR] = 0;
    }

    if (sem_created) {
        a[mutex] = 1;
        a[full] = 0;
        a[empty] = SIZE;

        snum.array = a;

        if (semctl(sid, 0, SETALL, snum) == -1) {
            perror("semctl");
            shmdt(buf);
            return 1;
        }
    }

    while (1) {
    wait_sem(sid, empty);

    printf("Enter Data: ");
    fflush(stdout);

    if (scanf("%d", &num) != 1) {
        signal_sem(sid, empty);
        break;
    }

    wait_sem(sid, mutex);

    buf[buf[WRITE_PTR]] = num;

    printf("Produced Data: %d\n", num);

    buf[WRITE_PTR] = (buf[WRITE_PTR] + 1) % SIZE;

    signal_sem(sid, mutex);
    signal_sem(sid, full);
}
    printf("[PRODUCER STOPPED]\n");

    shmdt(buf);

    return 0;
}

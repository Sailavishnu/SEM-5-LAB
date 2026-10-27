
#include "semaphore.h"

int main() {
    int shmid, sid, *buf, num;

    printf("=== CONSUMER STARTED ===\n\n");
    printf("Waiting for producer...\n");

    /* Wait for shared memory */
    while (1) {
        shmid = shmget(SHM_KEY, sizeof(int) * 12, 0666);

        if (shmid != -1)
            break;

        sleep(1);
    }

    /* Attach shared memory */
    buf = (int *)shmat(shmid, NULL, 0);

    if (buf == (void *)-1) {
        perror("shmat");
        return 1;
    }

    /* Wait for semaphore set */
    while (1) {
        sid = semget(SEM_KEY, 3, 0666);

        if (sid != -1)
            break;

        sleep(1);
    }

    printf("Connected to producer!\n");
    printf("Waiting for data...\n\n");

    /* Consume continuously */
    while (1) {
        wait_sem(sid, full);
        wait_sem(sid, mutex);

        num = buf[buf[READ_PTR]];

        printf("Consumed Data: %d\n", num);
        fflush(stdout);

        buf[READ_PTR] = (buf[READ_PTR] + 1) % SIZE;

        signal_sem(sid, mutex);
        signal_sem(sid, empty);
    }

    shmdt(buf);
    return 0;
}

#include "semaphore.h"

void wait_sem(int sid, int sno)
{
    struct sembuf s;

    s.sem_num = sno;
    s.sem_op = -1;
    s.sem_flg = SEM_UNDO;

    semop(sid, &s, 1);
}

void signal_sem(int sid, int sno)
{
    struct sembuf s;

    s.sem_num = sno;
    s.sem_op = 1;
    s.sem_flg = SEM_UNDO;

    semop(sid, &s, 1);
}
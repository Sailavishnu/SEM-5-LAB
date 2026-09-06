#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>

#define SIZE 10

#define SHM_KEY 2461
#define SEM_KEY 5777

/* Semaphore numbers */
#define mutex 0
#define full 1
#define empty 2

/* Positions inside shared memory */
#define WRITE_PTR 10
#define READ_PTR 11

union semaphore
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

void wait_sem(int sid, int sno);
void signal_sem(int sid, int sno);

#endif
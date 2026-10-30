#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int P2, P3, P4, P5;

    printf("P1: PID = %d, PPID = %d\n", getpid(), getppid());

    // P1 forks P2
    P2 = fork();
    if (P2 == 0) {
        // Inside P2 process
        printf("P2: PID = %d, PPID = %d\n", getpid(), getppid());

        // P2 forks P4
        P4 = fork();
        if (P4 == 0) {
            // Inside P4 process
            printf("P4: PID = %d, PPID = %d\n", getpid(), getppid());
            exit(0);
        } else {
            wait(NULL);  // P2 waits for P4
        }
        exit(0);
    } else {
        // Back in P1, fork P3
        P3 = fork();
        if (P3 == 0) {
            // Inside P3 process
            printf("P3: PID = %d, PPID = %d\n", getpid(), getppid());

            // P3 forks P5
            P5 = fork();
            if (P5 == 0) {
                // Inside P5 process
                printf("P5: PID = %d, PPID = %d\n", getpid(), getppid());
                exit(0);
            } else {
                wait(NULL);  // P3 waits for P5
            }
            exit(0);
        } else {
            // P1 waits for all children
            wait(NULL);
            wait(NULL);
        }
    }

    return 0;
}
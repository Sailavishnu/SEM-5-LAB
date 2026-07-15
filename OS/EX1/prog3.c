#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

int main() {
    int n;
    
    printf("Enter n: ");
    scanf("%d", &n);

    // P1 forks P2 (for primes)
    if (fork() == 0) {
        // Inside P2 process
        printf("\nIn P2 (PID = %d, PPID = %d)\n", getpid(), getppid());
        printf("First %d prime numbers: ", n);

        int count = 0, num = 2;
        while (count < n) {
            int prime = 1;
            // Check divisibility up to sqrt(num)
            for (int i = 2; i <= sqrt(num); i++) {
                if (num % i == 0) {
                    prime = 0;
                    break;
                }
            }
            if (prime) {
                printf("%d ", num);
                count++;
            }
            num++;
        }
        printf("\n");
        exit(0);
    } else {
        // Back in P1, fork P3 (for fibonacci)
        if (fork() == 0) {
            // Inside P3 process
            printf("In P3 (PID = %d, PPID = %d)\n", getpid(), getppid());
            printf("First %d fibonacci numbers: ", n);

            int a = 0, b = 1, c;
            // CORRECTED: was "i < num/2" - should be "i < n"
            for (int i = 0; i < n; i++) {
                printf("%d ", a);
                c = a + b;
                a = b;
                b = c;
            }
            printf("\n");
            exit(0);
        } else {
            // P1 waits for both children
            wait(NULL);
            wait(NULL);
            printf("\nBoth child processes completed.\n");
        }
    }

    return 0;
}
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

#define N 10

int main() {
    int arr[N];

    printf("Enter %d integers:\n", N);
    for (int i = 0; i < N; i++) scanf("%d", &arr[i]);

    pid_t c1 = fork();

    if (c1 == 0) {
        // Child 1 - median
        for (int i = 0; i < N-1; i++)
            for (int j = 0; j < N-i-1; j++)
                if (arr[j] > arr[j+1]) {
                    int t = arr[j]; arr[j] = arr[j+1]; arr[j+1] = t;
                }

        float med = (N % 2) ? arr[N/2] : (arr[N/2-1] + arr[N/2]) / 2.0;
        printf("Child1: Median = %.2f\n", med);
        exit(0);
    }

    pid_t c2 = fork();

    if (c2 == 0) {
        // Child 2 - variance
        float sum = 0, mean, var = 0;
        for (int i = 0; i < N; i++) sum += arr[i];
        mean = sum / N;

        for (int i = 0; i < N; i++) var += (arr[i]-mean)*(arr[i]-mean);
        var /= N;

        printf("Child2: Variance = %.2f\n", var);
        exit(0);
    }

    wait(NULL);
    wait(NULL);

    return 0;
}
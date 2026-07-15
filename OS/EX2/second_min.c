//         P1
//          |
//         P2
//          |
//         P3

// P3:
//     Read array from user
//     Write size and array to Pipe1

// P2:
//     Read from Pipe1
//     Remove all odd numbers
//     Write even numbers to Pipe2

// P1:
//     Read filtered array from Pipe2
//     Find and display the 2nd minimum (without sorting)


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <limits.h>

int main()
{
    int pipe1[2], pipe2[2];

    pipe(pipe1);
    pipe(pipe2);

    if (fork() == 0)
    {
        // P2

        if (fork() == 0)
        {
            // P3

            close(pipe1[0]);

            int n;

            printf("Enter size: ");
            scanf("%d", &n);

            int arr[n];

            printf("Enter %d elements: ", n);
            for (int i = 0; i < n; i++)
                scanf("%d", &arr[i]);

            printf("\nOriginal Array: ");
            for (int i = 0; i < n; i++)
                printf("%d ", arr[i]);
            printf("\n");

            write(pipe1[1], &n, sizeof(int));
            write(pipe1[1], arr, n * sizeof(int));

            close(pipe1[1]);
        }
        else
        {
            // P2

            close(pipe1[1]);
            close(pipe2[0]);

            int n;
            read(pipe1[0], &n, sizeof(int));

            int arr[n];
            read(pipe1[0], arr, n * sizeof(int));

            // Remove odd numbers (keep even numbers) in-place
            int j = 0;

            for (int i = 0; i < n; i++)
            {
                if (arr[i] % 2 == 0)
                {
                    arr[j++] = arr[i];
                }
            }

            n = j;

            write(pipe2[1], &n, sizeof(int));
            write(pipe2[1], arr, n * sizeof(int));

            close(pipe1[0]);
            close(pipe2[1]);

            wait(NULL);
        }
    }
    else
    {
        // P1

        close(pipe2[1]);

        int n;
        read(pipe2[0], &n, sizeof(int));

        int arr[n];
        read(pipe2[0], arr, n * sizeof(int));

        close(pipe2[0]);

        wait(NULL);

        printf("\nArray after removing odd numbers: ");
        for (int i = 0; i < n; i++)
            printf("%d ", arr[i]);

        printf("\n");

        if (n < 2)
        {
            printf("Second highest does not exist.\n");
            return 0;
        }

        int max1 = INT_MIN;
        int max2 = INT_MIN;

        for (int i = 0; i < n; i++)
        {
            if (arr[i] > max1)
            {
                max2 = max1;
                max1 = arr[i];
            }
            else if (arr[i] > max2 && arr[i] != max1)
            {
                max2 = arr[i];
            }
        }

        if (max2 == INT_MIN)
            printf("Second highest does not exist.\n");
        else
            printf("Second highest = %d\n", max2);
    }

    return 0;
}
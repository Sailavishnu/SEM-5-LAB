//         P1 (Parent)
//            |
//         fork()
//            |
//         P2 (Child)

// P2:
//   - Read two numbers from user
//   - Write both numbers into the pipe

// P1:
//   - Read the two numbers from the pipe
//   - Find and print the GCD


#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int gcd(int a, int b)
{
    if (b == 0)
        return a;
    return gcd(b, a % b);
}

int main()
{
    int fd[2];
    pipe(fd);

    pid_t pid = fork();

    if (pid == 0)
    {
        // P2 (Child)
        close(fd[0]); // Close read end

        int num1, num2;

        printf("Enter two numbers: ");
        scanf("%d %d", &num1, &num2);

        write(fd[1], &num1, sizeof(int));
        write(fd[1], &num2, sizeof(int));

        close(fd[1]);
    }
    else
    {
        // P1 (Parent)
        close(fd[1]); // Close write end

        int num1, num2;

        read(fd[0], &num1, sizeof(int));
        read(fd[0], &num2, sizeof(int));

        close(fd[0]);

        wait(NULL);

        printf("GCD of %d and %d = %d\n", num1, num2, gcd(num1, num2));
    }

    return 0;
}
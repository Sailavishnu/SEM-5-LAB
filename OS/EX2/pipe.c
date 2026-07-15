#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{
    int fd[2];

    pid_t pid;

    char *write_msg = "Hello from Parent";
    char read_msg[100];

    if (pipe(fd) == -1)
    {
        perror("Pipe creation failed");
        return 1;
    }

    pid = fork();

    if (pid < 0)
    {
        perror("Fork creation failed");
        return 1;
    }

    if (pid == 0)
    {
        // Child process

        close(fd[1]);

        int bytes_read = read(fd[0], read_msg, sizeof(read_msg));

        if (bytes_read > 0)
        {
            read_msg[bytes_read] = '\0';
            printf("Child Received: %s\n", read_msg);
        }

        close(fd[0]);
    }
    else
    {
        // Parent process

        close(fd[0]);

        printf("Parent sending: %s\n", write_msg);

        write(fd[1], write_msg, strlen(write_msg) + 1);

        close(fd[1]);

        wait(NULL);
    }

    return 0;
}
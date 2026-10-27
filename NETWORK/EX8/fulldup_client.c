
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>

int main()
{
    int s, n;
    char msg[100];
    struct sockaddr_in server = {0};
    struct timeval tv = {1, 0};
    socklen_t len = sizeof(server);

    s = socket(AF_INET, SOCK_DGRAM, 0);
    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    server.sin_family = AF_INET;
    server.sin_port = htons(8082);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Waiting for server...\n");

    while (1)
    {
        sendto(s, "CONNECT", 7, 0,
               (struct sockaddr *)&server, len);

        n = recvfrom(s, msg, 99, 0, NULL, NULL);

        if (n < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;
            perror("recvfrom");
            close(s);
            return 1;
        }

        msg[n] = '\0';
        if (strcmp(msg, "CONNECTED") == 0) break;
    }

    printf("Connected! Type QUIT to exit.\n");

    int r = fork();

    if (r == 0)
    {
        while (1)
        {
            n = recvfrom(s, msg, 99, 0, NULL, NULL);
            if (n <= 0) continue;
            msg[n] = '\0';

            if (strcmp(msg, "QUIT") == 0) {
                kill(getppid(), SIGTERM);
                _exit(0);
            }

            printf("\nServer: %s\n", msg);
        }
    }

    while (1)
    {
        printf("You: ");
        if (!fgets(msg, sizeof(msg), stdin)) break;

        msg[strcspn(msg, "\n")] = '\0';

        sendto(s, msg, strlen(msg), 0,
               (struct sockaddr *)&server, len);

        if (strcmp(msg, "QUIT") == 0) break;
    }

    kill(r, SIGTERM);
    close(s);
    wait(NULL);
    return 0;
}

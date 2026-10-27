
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

int main()
{
    int s, n, pid;
    char msg[100];
    struct sockaddr_in server = {0}, client, owner;
    socklen_t len = sizeof(client);

    signal(SIGCHLD, SIG_IGN);
    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8082);
    server.sin_addr.s_addr = INADDR_ANY;

    bind(s, (struct sockaddr *)&server, sizeof(server));
    printf("UDP Server waiting...\n");

    while (1)
    {
        n = recvfrom(s, msg, 99, 0,
                     (struct sockaddr *)&client, &len);
        if (n <= 0) continue;
        msg[n] = '\0';

        if (strcmp(msg, "CONNECT") != 0) continue;

        owner = client;
        sendto(s, "CONNECTED", 9, 0,
               (struct sockaddr *)&owner, len);

        pid = fork();

        if (pid == 0)
        {
            int r = fork();

            if (r == 0)
            {
                while (1)
                {
                    struct sockaddr_in from;
                    socklen_t flen = sizeof(from);

                    n = recvfrom(s, msg, 99, 0,
                                 (struct sockaddr *)&from, &flen);
                    if (n <= 0) continue;
                    msg[n] = '\0';

                    if (from.sin_port != owner.sin_port ||
                        from.sin_addr.s_addr != owner.sin_addr.s_addr)
                    {
                        if (strcmp(msg, "CONNECT") == 0)
                            sendto(s, "WAIT", 4, 0,
                                   (struct sockaddr *)&from, flen);
                        continue;
                    }

                    if (strcmp(msg, "QUIT") == 0)
                    {
                        kill(getppid(), SIGTERM);
                        _exit(0);
                    }

                    printf("\nClient: %s\n", msg);
                }
            }

            while (1)
            {
                printf("You: ");
                if (!fgets(msg, sizeof(msg), stdin)) break;

                sendto(s, msg, strlen(msg), 0,
                       (struct sockaddr *)&owner, len);

                if (strncmp(msg, "QUIT", 4) == 0) break;
            }

            kill(r, SIGTERM);
            close(s);
            _exit(0);
        }

        waitpid(pid, NULL, 0);
        printf("Client left. Waiting for next client...\n");
    }

    close(s);
    return 0;
}

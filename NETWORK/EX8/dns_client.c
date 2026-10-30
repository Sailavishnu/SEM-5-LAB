
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <errno.h>

int main()
{
    int s, n;
    char msg[100];
    struct sockaddr_in server = {0};
    struct timeval tv = {1, 0};
    socklen_t len = sizeof(server);

    s = socket(AF_INET, SOCK_DGRAM, 0);

    setsockopt(s, SOL_SOCKET, SO_RCVTIMEO,
               &tv, sizeof(tv));

    server.sin_family = AF_INET;
    server.sin_port = htons(8082);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Waiting for server...\n");

    while (1)
    {
        sendto(s, "CONNECT", 7, 0,
               (struct sockaddr *)&server, len);

        n = recvfrom(s, msg, 99, 0, NULL, NULL);

        if (n < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                continue;

            perror("recvfrom");
            close(s);
            return 1;
        }

        msg[n] = '\0';

        if (strcmp(msg, "CONNECTED") == 0)
            break;
    }

    printf("Connected! You can use DNS now.\n");

    while (1)
    {
        printf("Enter domain (-1 to exit): ");

        if (scanf("%99s", msg) != 1)
            break;

        sendto(s, msg, strlen(msg), 0,
               (struct sockaddr *)&server, len);

        if (strcmp(msg, "-1") == 0)
        {
            recvfrom(s, msg, 99, 0, NULL, NULL);
            printf("Disconnected\n");
            break;
        }

        n = recvfrom(s, msg, 99, 0, NULL, NULL);

        if (n < 0)
        {
            perror("recvfrom");
            break;
        }

        msg[n] = '\0';
        printf("IP Address: %s\n", msg);
    }

    close(s);
    return 0;
}

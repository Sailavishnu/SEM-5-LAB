#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define PORT 2121

int read_line(int s, char *b, int size)
{
    int i = 0;
    char ch;

    while (i < size - 1)
    {
        int n = recv(s, &ch, 1, 0);
        if (n <= 0) return n;
        if (ch == '\n') break;
        b[i++] = ch;
    }

    b[i] = '\0';
    return i;
}

int main(void)
{
    int s, n;
    char b[512], cmd[20];
    struct sockaddr_in addr = {0};

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("connect");
        close(s);
        return 1;
    }

    n = read_line(s, b, sizeof(b));
    if (n <= 0)
    {
        close(s);
        return 1;
    }

    printf("%s\n\n", b);

    while (1)
    {
        printf("ftp> ");
        fflush(stdout);

        if (!fgets(b, sizeof(b), stdin))
            break;

        if (!strchr(b, '\n'))
        {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
                ;
            printf("Command too long\n");
            continue;
        }

        sscanf(b, "%19s", cmd);

        if (send(s, b, strlen(b), 0) <= 0)
            break;

        n = read_line(s, b, sizeof(b));
        if (n <= 0)
            break;

        printf("%s\n", b);

        if (!strcmp(cmd, "DIR") && !strncmp(b, "200 ", 4))
        {
            int count = 0;

            while ((n = read_line(s, b, sizeof(b))) > 0)
            {
                if (!strcmp(b, "END"))
                    break;

                printf("  %s\n", b);
                count++;
            }

            if (count == 0)
                printf("  (empty)\n");
        }

        if (!strncmp(b, "221 ", 4))
            break;
    }

    close(s);
    return 0;
}
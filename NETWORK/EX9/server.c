#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>

#define PORT 2121

int read_line(int c, char *b, int size)
{
    int i = 0;
    char ch;

    while (i < size - 1)
    {
        int n = recv(c, &ch, 1, 0);
        if (n <= 0) return n;
        if (ch == '\n') break;
        b[i++] = ch;
    }

    b[i] = '\0';
    return i;
}

void send_line(int c, const char *msg)
{
    int len = strlen(msg), sent = 0;

    while (sent < len)
    {
        int n = send(c, msg + sent, len - sent, 0);
        if (n <= 0) return;
        sent += n;
    }
}

void reply(int c, const char *msg)
{
    printf("[SERVER RESPONSE] %s", msg);
    fflush(stdout);
    send_line(c, msg);
}

void handle(int c)
{
    char b[512], cmd[20], a[100];
    int n, user_ok = 0, login = 0;

    reply(c, "220 - Server ready\n");

    while ((n = read_line(c, b, sizeof(b))) > 0)
    {
        printf("\n[CLIENT REQUEST] %s\n", b);
        fflush(stdout);

        cmd[0] = a[0] = '\0';
        sscanf(b, "%19s %99s", cmd, a);

        if (!strcmp(cmd, "EXIT"))
        {
            reply(c, "221 - Connection closed\n");
            break;
        }

        if (!login)
        {
            if (!strcmp(cmd, "USER"))
            {
                user_ok = !strcmp(a, "system");

                reply(c, user_ok
                    ? "331 - Enter password\n"
                    : "530 - Invalid username\n");
            }
            else if (!strcmp(cmd, "PASS"))
            {
                if (!user_ok)
                    reply(c, "503 - Send USER first\n");
                else if (!strcmp(a, "system123"))
                {
                    login = 1;
                    reply(c, "230 - Login successful\n");
                }
                else
                {
                    user_ok = 0;
                    reply(c, "530 - Login failed\n");
                }
            }
            else
                reply(c, "530 - Login first\n");

            continue;
        }

        if (!strcmp(cmd, "USER") || !strcmp(cmd, "PASS"))
        {
            reply(c, "530 - Already logged in\n");
        }
        else if (!strcmp(cmd, "DIR"))
        {
            DIR *dir = opendir("ftp_root");
            struct dirent *e;
            int count = 0;

            if (!dir)
            {
                reply(c, "550 - Directory error\n");
                continue;
            }

            reply(c, "200 - Files listed successfully\n");

            while ((e = readdir(dir)) != NULL)
            {
                if (e->d_name[0] != '.')
                {
                    char line[256];

                    snprintf(line, sizeof(line), "%.254s\n", e->d_name);
                    printf("[SERVER DATA] %s", line);
                    fflush(stdout);
                    send_line(c, line);
                    count++;
                }
            }

            if (count == 0)
            {
                printf("[SERVER DATA] (empty)\n");
                fflush(stdout);
            }

            send_line(c, "END\n");
            closedir(dir);
        }
        else if (!strcmp(cmd, "CREATE") ||
                 !strcmp(cmd, "DELETE"))
        {
            char path[150];

            if (!a[0] || strchr(a, '/') || strstr(a, ".."))
                reply(c, "550 - Invalid filename\n");
            else
            {
                snprintf(path, sizeof(path), "ftp_root/%s", a);

                if (!strcmp(cmd, "CREATE"))
                {
                    FILE *f = fopen(path, "wx");

                    if (f)
                    {
                        fclose(f);
                        reply(c, "250 - File created\n");
                    }
                    else
                        reply(c, "550 - Create failed\n");
                }
                else
                    reply(c, remove(path) == 0
                        ? "250 - File deleted\n"
                        : "550 - Delete failed\n");
            }
        }
        else
            reply(c, "502 - Invalid command\n");
    }

    close(c);
}

int main(void)
{
    int s, c, opt = 1;
    struct sockaddr_in addr = {0};

    mkdir("ftp_root", 0755);
    signal(SIGCHLD, SIG_IGN);

    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0)
    {
        perror("socket");
        return 1;
    }

    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0 ||
        listen(s, 5) < 0)
    {
        perror("bind/listen");
        close(s);
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1)
    {
        c = accept(s, NULL, NULL);
        if (c < 0) continue;

        if (fork() == 0)
        {
            close(s);
            handle(c);
            exit(0);
        }

        close(c);
    }
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define DIRNAME "ftp_store"

void handle(int client) {
    char buf[1024], cmd[64], arg[256], out[2048], path[512];
    int n, user_ok = 0, logged = 0;
    struct stat st;

    send(client, "220 Server ready\n", 17, 0);

    while ((n = recv(client, buf, sizeof(buf) - 1, 0)) > 0) {
        buf[n] = '\0';
        cmd[0] = arg[0] = '\0';
        sscanf(buf, "%63s %255s", cmd, arg);
        printf("[cmd] %s %s\n", cmd, arg);

        if (strcmp(cmd, "USER") == 0) {
            if (strcmp(arg, "admin") == 0) {
                user_ok = 1;
                strcpy(out, "331 Username OK, password required\n");
                printf("331 User OK, waiting for password\n");
            } else {
                user_ok = 0;
                strcpy(out, "530 Invalid username\n");
                printf("530 Invalid username\n");
            }
        }
        else if (strcmp(cmd, "PASS") == 0) {
            if (user_ok && strcmp(arg, "admin123") == 0) {
                logged = 1;
                strcpy(out, "230 Login successful\n");
                printf("230 Client connected (logged in)\n");
            } else {
                strcpy(out, "530 Login incorrect\n");
                printf("530 Login incorrect\n");
            }
        }
        else if (!logged && (strcmp(cmd, "STOR") == 0 ||
                 strcmp(cmd, "RETR") == 0 || strcmp(cmd, "DIR") == 0)) {
            strcpy(out, "530 Please login first (USER/PASS)\n");
            printf("530 Not logged in\n");
        }
        else if (strcmp(cmd, "STOR") == 0) {
            if (arg[0] == '\0' || strchr(arg, '/')) {
                strcpy(out, "501 Invalid filename\n");
            } else {
                mkdir(DIRNAME, 0777);              /* create dir if not exist */
                snprintf(path, sizeof(path), "%s/%s", DIRNAME, arg);
                int fd = open(path, O_CREAT | O_WRONLY, 0666); /* empty copy */
                if (fd < 0) {
                    strcpy(out, "550 Cannot create file\n");
                } else {
                    close(fd);
                    strcpy(out, "150 Ready to receive\n226 File received successfully\n");
                    printf("150 Ready to receive\n226 File received: %s\n", arg);
                }
            }
        }
        else if (strcmp(cmd, "RETR") == 0) {
            snprintf(path, sizeof(path), "%s/%s", DIRNAME, arg);
            if (arg[0] != '\0' && !strchr(arg, '/') && stat(path, &st) == 0) {
                strcpy(out, "226 File retrieved successfully\n");
                printf("226 File retrieved: %s\n", arg);
            } else {
                strcpy(out, "550 File not found\n");
                printf("550 File not found: %s\n", arg);
            }
        }
        else if (strcmp(cmd, "DIR") == 0) {
            strcpy(out, "150 Directory listing\n");
            DIR *d = opendir(DIRNAME);
            if (d) {
                struct dirent *e;
                while ((e = readdir(d)) != NULL)
                    if (e->d_name[0] != '.') {
                        strcat(out, e->d_name);
                        strcat(out, "\n");
                    }
                closedir(d);
            }
            strcat(out, "226 Directory send OK\n");
            printf("226 Directory sent\n");
        }
        else {
            strcpy(out, "500 Unknown command\n");
            printf("500 Unknown command\n");
        }

        send(client, out, strlen(out), 0);
    }

    printf("Client disconnected.\n");
    close(client);
}

int main() {
    int sock, client;
    struct sockaddr_in server, caddr;
    socklen_t len = sizeof(caddr);

    signal(SIGCHLD, SIG_IGN);              /* no zombie children */

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) { perror("Socket error"); return 1; }

    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Bind error"); close(sock); return 1;
    }
    if (listen(sock, 5) < 0) {
        perror("Listen error"); close(sock); return 1;
    }

    printf("220 Server ready, waiting for clients...\n");

    while (1) {
        client = accept(sock, (struct sockaddr *)&caddr, &len);
        if (client < 0) { perror("Accept error"); continue; }
        printf("Client connected: %s\n", inet_ntoa(caddr.sin_addr));

        if (fork() == 0) {                 /* child serves this client */
            close(sock);
            handle(client);
            exit(0);
        }
        close(client);                     /* parent keeps accepting */
    }
}
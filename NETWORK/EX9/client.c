#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8066
#define SIZE 1024
#define BUF 4096

int send_all(int sock, const void *data, size_t len)
{
    const char *p = data;
    while (len > 0) {
        ssize_t n = send(sock, p, len, 0);
        if (n <= 0) return -1;
        p += n;
        len -= n;
    }
    return 0;
}

int recv_all(int sock, void *data, size_t len)
{
    char *p = data;
    while (len > 0) {
        ssize_t n = recv(sock, p, len, 0);
        if (n <= 0) return -1;
        p += n;
        len -= n;
    }
    return 0;
}

int recv_line(int sock, char *buf, size_t size)
{
    size_t i = 0;
    char ch;
    while (i < size - 1) {
        ssize_t n = recv(sock, &ch, 1, 0);
        if (n <= 0) return (int)n;
        if (ch == '\n') break;
        if (ch != '\r') buf[i++] = ch;
    }
    buf[i] = '\0';
    return 1;
}

int valid_name(const char *name)
{
    if (!name || !*name || !strcmp(name, ".") ||
        !strcmp(name, "..")) return 0;
    for (int i = 0; name[i]; i++) {
        if (!isalnum((unsigned char)name[i]) &&
            name[i] != '.' && name[i] != '_' &&
            name[i] != '-') return 0;
    }
    return 1;
}

void list_files(int sock)
{
    char line[SIZE];
    send_all(sock, "DIR\n", 4);
    if (recv_line(sock, line, sizeof(line)) != 1) return;
    puts(line);
    if (strncmp(line, "150 ", 4)) return;
    while (recv_line(sock, line, sizeof(line)) == 1) {
        if (!strcmp(line, ".")) break;
        puts(line);
    }
    if (recv_line(sock, line, sizeof(line)) == 1) puts(line);
}

void upload_file(int sock, char *name)
{
    char request[SIZE], response[SIZE], buf[BUF];
    if (!valid_name(name)) {
        puts("Invalid filename.");
        return;
    }
    FILE *fp = fopen(name, "rb");
    if (!fp) {
        perror("File");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    if (size < 0) {
        fclose(fp);
        return;
    }
    snprintf(request, sizeof(request), "STOR %s %ld\n", name, size);
    if (send_all(sock, request, strlen(request)) < 0 ||
        recv_line(sock, response, sizeof(response)) != 1) {
        fclose(fp);
        return;
    }
    puts(response);
    if (strncmp(response, "150 ", 4)) {
        fclose(fp);
        return;
    }
    size_t count;
    while ((count = fread(buf, 1, sizeof(buf), fp)) > 0) {
        if (send_all(sock, buf, count) < 0) break;
    }
    fclose(fp);
    if (recv_line(sock, response, sizeof(response)) == 1)
        puts(response);
}

void download_file(int sock, char *name)
{
    char request[SIZE], response[SIZE], buf[BUF];
    unsigned long long size;
    if (!valid_name(name)) {
        puts("Invalid filename.");
        return;
    }
    snprintf(request, sizeof(request), "RETR %s\n", name);
    if (send_all(sock, request, strlen(request)) < 0 ||
        recv_line(sock, response, sizeof(response)) != 1) return;
    if (sscanf(response, "150 SIZE %llu", &size) != 1) {
        puts(response);
        return;
    }
    FILE *fp = fopen(name, "wb");
    if (!fp) {
        send_all(sock, "CANCEL\n", 7);
        recv_line(sock, response, sizeof(response));
        return;
    }
    send_all(sock, "READY\n", 6);
    while (size > 0) {
        size_t chunk = size > BUF ? BUF : (size_t)size;
        if (recv_all(sock, buf, chunk) < 0) {
            fclose(fp);
            remove(name);
            return;
        }
        fwrite(buf, 1, chunk, fp);
        size -= chunk;
    }
    fclose(fp);
    if (recv_line(sock, response, sizeof(response)) == 1)
        puts(response);
}

int main(void)
{
    int sock;
    struct sockaddr_in server;
    char user[50], pass[50], line[SIZE], request[SIZE];
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server.sin_addr);
    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return 1;
    }
    if (recv_line(sock, line, sizeof(line)) != 1) {
        close(sock);
        return 1;
    }
    puts(line);
    printf("USER ");
    if (!fgets(user, sizeof(user), stdin)) {
        close(sock);
        return 1;
    }
    user[strcspn(user, "\n")] = '\0';
    snprintf(request, sizeof(request), "USER %s\n", user);
    send_all(sock, request, strlen(request));
    if (recv_line(sock, line, sizeof(line)) != 1) {
        close(sock);
        return 1;
    }
    puts(line);
    printf("PASS ");
    if (!fgets(pass, sizeof(pass), stdin)) {
        close(sock);
        return 1;
    }
    pass[strcspn(pass, "\n")] = '\0';
    snprintf(request, sizeof(request), "PASS %s\n", pass);
    memset(pass, 0, sizeof(pass));
    send_all(sock, request, strlen(request));
    if (recv_line(sock, line, sizeof(line)) != 1) {
        close(sock);
        return 1;
    }
    puts(line);
    if (strncmp(line, "230 ", 4)) {
        close(sock);
        return 1;
    }
    puts("DIR | STOR filename | RETR filename | QUIT");
    while (1) {
        printf("ftp> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = '\0';
        if (!strcmp(line, "DIR")) {
            list_files(sock);
        } else if (!strncmp(line, "STOR ", 5)) {
            upload_file(sock, line + 5);
        } else if (!strncmp(line, "RETR ", 5)) {
            download_file(sock, line + 5);
        } else if (!strcmp(line, "QUIT")) {
            send_all(sock, "QUIT\n", 5);
            if (recv_line(sock, line, sizeof(line)) == 1)
                puts(line);
            break;
        } else {
            puts("Invalid command.");
        }
    }
    close(sock);
    return 0;
}

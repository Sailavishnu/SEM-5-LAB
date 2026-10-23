#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <dirent.h>

#define PORT 8066
#define SIZE 1024
#define BUF 4096
#define MAX_FILE (1024ULL * 1024ULL * 1024ULL)

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

void reply(int sock, const char *msg)
{
    send_all(sock, msg, strlen(msg));
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

int authenticate(const char *user, const char *pass)
{
    FILE *fp = fopen("users.txt", "r");
    char u[50], p[50];
    if (!fp) return 0;
    while (fscanf(fp, "%49s %49s", u, p) == 2) {
        if (!strcmp(user, u) && !strcmp(pass, p)) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void list_files(int client)
{
    DIR *dir = opendir("server_files");
    struct dirent *entry;
    if (!dir) {
        reply(client, "550 Cannot open directory.\n");
        return;
    }
    reply(client, "150 Directory listing.\n");
    while ((entry = readdir(dir)) != NULL) {
        if (!strcmp(entry->d_name, ".") ||
            !strcmp(entry->d_name, "..")) continue;
        send_all(client, entry->d_name, strlen(entry->d_name));
        send_all(client, "\n", 1);
    }
    closedir(dir);
    reply(client, ".\n226 Directory listing complete.\n");
}

void receive_file(int client, char *name, unsigned long long size)
{
    char path[SIZE], buf[BUF];
    if (!valid_name(name) || size > MAX_FILE) {
        reply(client, "550 Invalid file or size.\n");
        return;
    }
    snprintf(path, sizeof(path), "server_files/%s", name);
    FILE *fp = fopen(path, "wb");
    if (!fp) {
        reply(client, "550 Cannot create file.\n");
        return;
    }
    reply(client, "150 Ready to receive file.\n");
    while (size > 0) {
        size_t chunk = size > BUF ? BUF : (size_t)size;
        if (recv_all(client, buf, chunk) < 0) {
            fclose(fp);
            remove(path);
            return;
        }
        if (fwrite(buf, 1, chunk, fp) != chunk) {
            fclose(fp);
            remove(path);
            reply(client, "451 Write failed.\n");
            return;
        }
        size -= chunk;
    }
    fclose(fp);
    reply(client, "226 Transfer complete.\n");
    printf("Uploaded: %s\n", name);
}

void send_file(int client, char *name)
{
    char path[SIZE], buf[BUF], line[SIZE];
    FILE *fp;
    unsigned long long size;
    if (!valid_name(name)) {
        reply(client, "550 Invalid filename.\n");
        return;
    }
    snprintf(path, sizeof(path), "server_files/%s", name);
    fp = fopen(path, "rb");
    if (!fp) {
        reply(client, "550 File not found.\n");
        return;
    }
    fseek(fp, 0, SEEK_END);
    long end = ftell(fp);
    rewind(fp);
    if (end < 0) {
        fclose(fp);
        reply(client, "451 Cannot read file.\n");
        return;
    }
    size = (unsigned long long)end;
    snprintf(line, sizeof(line), "150 SIZE %llu\n", size);
    reply(client, line);
    if (recv_line(client, line, sizeof(line)) != 1 ||
        strcmp(line, "READY")) {
        fclose(fp);
        reply(client, "426 Transfer aborted.\n");
        return;
    }
    while (size > 0) {
        size_t chunk = size > BUF ? BUF : (size_t)size;
        size_t n = fread(buf, 1, chunk, fp);
        if (n != chunk || send_all(client, buf, n) < 0) {
            fclose(fp);
            return;
        }
        size -= chunk;
    }
    fclose(fp);
    reply(client, "226 Transfer complete.\n");
    printf("Downloaded: %s\n", name);
}

void handle_client(int client)
{
    char line[SIZE], user[50], pass[50], name[256];
    unsigned long long size;
    reply(client, "220 FTP Server Ready.\n");
    if (recv_line(client, line, sizeof(line)) != 1 ||
        sscanf(line, "USER %49s", user) != 1) {
        reply(client, "530 Send USER first.\n");
        return;
    }
    reply(client, "331 Enter password.\n");
    if (recv_line(client, line, sizeof(line)) != 1 ||
        sscanf(line, "PASS %49s", pass) != 1) {
        reply(client, "530 Invalid password command.\n");
        return;
    }
    if (!authenticate(user, pass)) {
        reply(client, "530 Login incorrect.\n");
        return;
    }
    memset(pass, 0, sizeof(pass));
    reply(client, "230 Login successful.\n");
    printf("Logged in: %s\n", user);
    while (recv_line(client, line, sizeof(line)) == 1) {
        if (!strcmp(line, "DIR")) {
            list_files(client);
        } else if (!strcmp(line, "QUIT")) {
            reply(client, "221 Goodbye.\n");
            break;
        } else if (!strncmp(line, "STOR ", 5)) {
            if (sscanf(line + 5, "%255s %llu", name, &size) == 2)
                receive_file(client, name, size);
            else
                reply(client, "501 Invalid STOR command.\n");
        } else if (!strncmp(line, "RETR ", 5)) {
            if (sscanf(line + 5, "%255s", name) == 1)
                send_file(client, name);
            else
                reply(client, "501 Invalid RETR command.\n");
        } else {
            reply(client, "502 Unknown command.\n");
        }
    }
    printf("Disconnected: %s\n", user);
}

int main(void)
{
    int server, client, opt = 1;
    struct sockaddr_in addr;
    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    mkdir("server_files", 0755);
    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("socket");
        return 1;
    }
    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(PORT);
    if (bind(server, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(server);
        return 1;
    }
    if (listen(server, 10) < 0) {
        perror("listen");
        close(server);
        return 1;
    }
    printf("FTP server running on port %d\n", PORT);
    while (1) {
        client = accept(server, NULL, NULL);
        if (client < 0) continue;
        pid_t pid = fork();
        if (pid == 0) {
            close(server);
            handle_client(client);
            close(client);
            _exit(0);
        }
        close(client);
    }
    close(server);
    return 0;
}

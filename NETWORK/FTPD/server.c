#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

#define PORT 8066
#define BUFFER_SIZE 4096
#define LINE_SIZE 1024
#define MAX_FILE_SIZE (1024ULL * 1024ULL * 1024ULL)

#ifndef NAME_MAX
#define NAME_MAX 255
#endif

/* Send all bytes */
int send_all(int sock, const void *data, size_t len)
{
    const char *p = data;

    while (len > 0)
    {
        ssize_t n = send(sock, p, len, 0);

        if (n <= 0)
            return -1;

        p += n;
        len -= (size_t)n;
    }

    return 0;
}

/* Receive an exact number of bytes */
int recv_all(int sock, void *data, size_t len)
{
    char *p = data;

    while (len > 0)
    {
        ssize_t n = recv(sock, p, len, 0);

        if (n <= 0)
            return -1;

        p += n;
        len -= (size_t)n;
    }

    return 0;
}

/* Receive a newline-terminated command */
int recv_line(int sock, char *buffer, size_t size)
{
    size_t i = 0;
    char ch;

    if (size < 2)
        return -1;

    while (1)
    {
        ssize_t n = recv(sock, &ch, 1, 0);

        if (n == 0)
            return 0;

        if (n < 0)
            return -1;

        if (ch == '\n')
        {
            buffer[i] = '\0';
            return 1;
        }

        if (ch == '\r')
            continue;

        if (i >= size - 1)
        {
            while (ch != '\n')
            {
                n = recv(sock, &ch, 1, 0);
                if (n <= 0)
                    return -1;
            }

            buffer[0] = '\0';
            return -1;
        }

        buffer[i++] = ch;
    }
}

/* Send a text response */
void reply(int sock, const char *message)
{
    send_all(sock, message, strlen(message));
}

/* Validate filenames */
int valid_filename(const char *name)
{
    if (name == NULL || name[0] == '\0')
        return 0;

    if (strcmp(name, ".") == 0 ||
        strcmp(name, "..") == 0)
        return 0;

    for (size_t i = 0; name[i] != '\0'; i++)
    {
        unsigned char ch = (unsigned char)name[i];

        if (!isalnum(ch) && ch != '.' &&
            ch != '_' && ch != '-')
            return 0;
    }

    return 1;
}

/* Check credentials against users.txt */
int authenticate(const char *username, const char *password)
{
    FILE *fp = fopen("users.txt", "r");
    char user[50], pass[50];

    if (fp == NULL)
    {
        perror("users.txt");
        return 0;
    }

    while (fscanf(fp, "%49s %49s", user, pass) == 2)
    {
        if (strcmp(username, user) == 0 &&
            strcmp(password, pass) == 0)
        {
            fclose(fp);
            return 1;
        }
    }

    fclose(fp);
    return 0;
}

/* DIR: display server files */
void handle_DIR(int client)
{
    DIR *dir = opendir("server_files");

    if (dir == NULL)
    {
        reply(client, "550 Cannot open server directory.\n");
        return;
    }

    reply(client, "150 Here comes the directory listing.\n");

    struct dirent *entry;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 ||
            strcmp(entry->d_name, "..") == 0)
            continue;

        if (send_all(client, entry->d_name,
                     strlen(entry->d_name)) < 0 ||
            send_all(client, "\n", 1) < 0)
        {
            closedir(dir);
            return;
        }
    }

    closedir(dir);

    reply(client, ".\n226 Directory Listing  complete.\n");
}

/* STOR: receive a file from the client */
void handle_stor(int client, const char *filename,
                 unsigned long long file_size)
{
    char path[LINE_SIZE];
    char buffer[BUFFER_SIZE];

    if (!valid_filename(filename))
    {
        reply(client, "550 Invalid filename.\n");
        return;
    }

    if (file_size > MAX_FILE_SIZE)
    {
        reply(client, "552 File too large.\n");
        return;
    }

    int n = snprintf(path, sizeof(path),
                     "server_files/%s", filename);

    if (n < 0 || (size_t)n >= sizeof(path))
    {
        reply(client, "550 Invalid file path.\n");
        return;
    }

    FILE *fp = fopen(path, "wb");

    if (fp == NULL)
    {
        reply(client, "550 Cannot create file.\n");
        return;
    }

    reply(client, "150 Ready to receive file.\n");

    unsigned long long remaining = file_size;
    int write_error = 0;

    while (remaining > 0)
    {
        size_t chunk = remaining > BUFFER_SIZE
                       ? BUFFER_SIZE : (size_t)remaining;

        if (recv_all(client, buffer, chunk) < 0)
        {
            fclose(fp);
            remove(path);
            return;
        }

        if (!write_error &&
            fwrite(buffer, 1, chunk, fp) != chunk)
        {
            write_error = 1;
        }

        remaining -= chunk;
    }

    if (fclose(fp) != 0)
        write_error = 1;

    if (write_error)
        reply(client, "451 File write failed.\n");
    else
        reply(client, "226 Transfer complete.\n");

    if (!write_error)
        printf("Uploaded: %s\n", filename);
}

/* RETR: send a file to the client */
void handle_retr(int client, const char *filename)
{
    char path[LINE_SIZE];
    char buffer[BUFFER_SIZE];
    char response[LINE_SIZE];
    char line[LINE_SIZE];

    if (!valid_filename(filename))
    {
        reply(client, "550 Invalid filename.\n");
        return;
    }

    int n = snprintf(path, sizeof(path),
                     "server_files/%s", filename);

    if (n < 0 || (size_t)n >= sizeof(path))
    {
        reply(client, "550 Invalid file path.\n");
        return;
    }

    FILE *fp = fopen(path, "rb");

    if (fp == NULL)
    {
        reply(client, "550 File not found.\n");
        return;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        reply(client, "451 Cannot read file.\n");
        return;
    }

    long end = ftell(fp);

    if (end < 0 || fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        reply(client, "451 Cannot read file.\n");
        return;
    }

    unsigned long long file_size = (unsigned long long)end;

    snprintf(response, sizeof(response),
             "150 SIZE %llu\n", file_size);

    if (send_all(client, response, strlen(response)) < 0)
    {
        fclose(fp);
        return;
    }

    /* Wait until client is ready */
    if (recv_line(client, line, sizeof(line)) != 1)
    {
        fclose(fp);
        return;
    }

    if (strcmp(line, "READY") != 0)
    {
        fclose(fp);
        reply(client, "426 Transfer aborted.\n");
        return;
    }

    unsigned long long remaining = file_size;

    while (remaining > 0)
    {
        size_t chunk = remaining > BUFFER_SIZE
                       ? BUFFER_SIZE : (size_t)remaining;

        size_t bytes = fread(buffer, 1, chunk, fp);

        if (bytes != chunk)
        {
            fclose(fp);
            return;
        }

        if (send_all(client, buffer, bytes) < 0)
        {
            fclose(fp);
            return;
        }

        remaining -= chunk;
    }

    fclose(fp);

    reply(client, "226 Transfer complete.\n");
    printf("Downloaded by client: %s\n", filename);
}

/* Handle one authenticated client */
void handle_client(int client)
{
    char line[LINE_SIZE];
    char username[50] = "";
    char password[50];

    reply(client, "220 FTP Server Ready.\n");

    /* USER command */
    if (recv_line(client, line, sizeof(line)) != 1 ||
        sscanf(line, "USER %49s", username) != 1)
    {
        reply(client, "530 Please send USER first.\n");
        return;
    }

    reply(client, "331 Enter password\n");

    /* PASS command */
    if (recv_line(client, line, sizeof(line)) != 1 ||
        sscanf(line, "PASS %49s", password) != 1)
    {
        reply(client, "530 Invalid password command.\n");
        return;
    }

    if (!authenticate(username, password))
    {
        reply(client, "530 Login incorrect.\n");
        printf("Login failed: %s\n", username);
        return;
    }

    memset(password, 0, sizeof(password));

    reply(client, "230 Login successful. Access granted.\n");
    printf("Logged in: %s\n", username);

    /* Process commands */
    while (1)
    {
        if (recv_line(client, line, sizeof(line)) != 1)
            break;

        if (strcmp(line, "DIR") == 0)
        {
            handle_DIR(client);
        }
        else if (strcmp(line, "QUIT") == 0)
        {
            reply(client, "221 Goodbye. Connection closed.\n");
            break;
        }
        else if (strncmp(line, "STOR ", 5) == 0)
        {
            char filename[NAME_MAX + 1];
            unsigned long long file_size;

            if (sscanf(line + 5, "%255s %llu",
                       filename, &file_size) != 2)
            {
                reply(client, "501 Invalid STOR command.\n");
                continue;
            }

            handle_stor(client, filename, file_size);
        }
        else if (strncmp(line, "RETR ", 5) == 0)
        {
            char filename[NAME_MAX + 1];

            if (sscanf(line + 5, "%255s", filename) != 1)
            {
                reply(client, "501 Invalid RETR command.\n");
                continue;
            }

            handle_retr(client, filename);
        }
        else
        {
            reply(client, "502 Command not implemented.\n");
        }
    }

    printf("Disconnected: %s\n", username);
}

/* Main concurrent server */
int main(void)
{
    int server, client;
    int opt = 1;
    struct sockaddr_in address;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    if (mkdir("server_files", 0755) < 0)
    {
        struct stat st;

        if (stat("server_files", &st) < 0 ||
            !S_ISDIR(st.st_mode))
        {
            perror("server_files");
            return 1;
        }
    }

    server = socket(AF_INET, SOCK_STREAM, 0);

    if (server < 0)
    {
        perror("socket");
        return 1;
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(PORT);

    if (bind(server, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind");
        close(server);
        return 1;
    }

    if (listen(server, 10) < 0)
    {
        perror("listen");
        close(server);
        return 1;
    }

    printf("[+] Concurrent FTP Server started on port %d.\n", PORT);
    printf("[+] Waiting for clients...\n");

    while (1)
    {
        client = accept(server, NULL, NULL);

        if (client < 0)
            continue;

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            close(client);
            continue;
        }

        if (pid == 0)
        {
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

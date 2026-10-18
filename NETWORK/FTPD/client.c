#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <termios.h>
#include <limits.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8066
#define BUFFER_SIZE 4096
#define LINE_SIZE 1024

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

/* Read password and display asterisks */
int read_password(char *password, size_t size)
{
    struct termios oldt, newt;
    size_t i = 0;
    int ch;

    if (tcgetattr(STDIN_FILENO, &oldt) != 0)
    {
        printf("Password masking requires a terminal.\n");
        return -1;
    }

    newt = oldt;
    newt.c_lflag &= ~(ECHO | ICANON);
    newt.c_cc[VMIN] = 1;
    newt.c_cc[VTIME] = 0;

    printf("PASS ");
    fflush(stdout);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &newt) != 0)
        return -1;

    while ((ch = getchar()) != '\n' && ch != EOF)
    {
        if (ch == 127 || ch == '\b')
        {
            if (i > 0)
            {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        }
        else if (ch >= 32 && ch <= 126 && i < size - 1)
        {
            password[i++] = (char)ch;
            putchar('*');
            fflush(stdout);
        }
    }

    password[i] = '\0';

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    printf("\n");

    return ch == EOF ? -1 : 0;
}

/* DIR command */
void DIR_files(int sock)
{
    char line[LINE_SIZE];

    if (send_all(sock, "DIR\n", 5) < 0)
        return;

    if (recv_line(sock, line, sizeof(line)) != 1)
    {
        printf("Connection lost.\n");
        return;
    }

    printf("%s\n", line);

    if (strncmp(line, "150 ", 4) != 0)
        return;

    while (recv_line(sock, line, sizeof(line)) == 1)
    {
        if (strcmp(line, ".") == 0)
            break;

        printf("%s\n", line);
    }

    if (recv_line(sock, line, sizeof(line)) == 1)
        printf("%s\n", line);
}

/* STOR command: upload a file */
void upload_file(int sock, const char *filename)
{
    char request[LINE_SIZE];
    char response[LINE_SIZE];
    char buffer[BUFFER_SIZE];

    if (!valid_filename(filename))
    {
        printf("Invalid filename.\n");
        return;
    }

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL)
    {
        perror("Local file");
        return;
    }

    if (fseek(fp, 0, SEEK_END) != 0)
    {
        fclose(fp);
        printf("Cannot determine file size.\n");
        return;
    }

    long end = ftell(fp);

    if (end < 0 || fseek(fp, 0, SEEK_SET) != 0)
    {
        fclose(fp);
        printf("Cannot read file.\n");
        return;
    }

    unsigned long long file_size = (unsigned long long)end;

    int n = snprintf(request, sizeof(request),
                     "STOR %s %llu\n", filename, file_size);

    if (n < 0 || (size_t)n >= sizeof(request) ||
        send_all(sock, request, strlen(request)) < 0)
    {
        fclose(fp);
        printf("Could not send request.\n");
        return;
    }

    if (recv_line(sock, response, sizeof(response)) != 1)
    {
        fclose(fp);
        printf("Connection lost.\n");
        return;
    }

    printf("%s\n", response);

    if (strncmp(response, "150 ", 4) != 0)
    {
        fclose(fp);
        return;
    }

    unsigned long long remaining = file_size;
    int failed = 0;

    while (remaining > 0)
    {
        size_t chunk = remaining > BUFFER_SIZE
                       ? BUFFER_SIZE : (size_t)remaining;

        size_t bytes = fread(buffer, 1, chunk, fp);

        if (bytes != chunk ||
            send_all(sock, buffer, bytes) < 0)
        {
            failed = 1;
            break;
        }

        remaining -= chunk;
    }

    if (fclose(fp) != 0)
        failed = 1;

    if (failed)
    {
        printf("Upload interrupted. Reconnect before continuing.\n");
        return;
    }

    if (recv_line(sock, response, sizeof(response)) == 1)
        printf("%s\n", response);
    else
        printf("Connection lost during upload.\n");
}

/* RETR command: download a file */
void download_file(int sock, const char *filename)
{
    char request[LINE_SIZE];
    char response[LINE_SIZE];
    char buffer[BUFFER_SIZE];

    if (!valid_filename(filename))
    {
        printf("Invalid filename.\n");
        return;
    }

    int n = snprintf(request, sizeof(request),
                     "RETR %s\n", filename);

    if (n < 0 || (size_t)n >= sizeof(request) ||
        send_all(sock, request, strlen(request)) < 0)
    {
        printf("Could not send request.\n");
        return;
    }

    if (recv_line(sock, response, sizeof(response)) != 1)
    {
        printf("Connection lost.\n");
        return;
    }

    unsigned long long file_size;

    if (sscanf(response, "150 SIZE %llu", &file_size) != 1)
    {
        printf("%s\n", response);
        return;
    }

    printf("150 File found. Size: %llu bytes\n", file_size);

    FILE *fp = fopen(filename, "wb");

    if (fp == NULL)
    {
        perror("Local file");

        send_all(sock, "CANCEL\n", 7);

        if (recv_line(sock, response, sizeof(response)) == 1)
            printf("%s\n", response);

        return;
    }

    if (send_all(sock, "READY\n", 6) < 0)
    {
        fclose(fp);
        remove(filename);
        printf("Connection lost.\n");
        return;
    }

    unsigned long long remaining = file_size;
    int failed = 0;

    while (remaining > 0)
    {
        size_t chunk = remaining > BUFFER_SIZE
                       ? BUFFER_SIZE : (size_t)remaining;

        if (recv_all(sock, buffer, chunk) < 0)
        {
            failed = 1;
            break;
        }

        if (fwrite(buffer, 1, chunk, fp) != chunk)
            failed = 1;

        remaining -= chunk;
    }

    if (fclose(fp) != 0)
        failed = 1;

    if (failed)
    {
        remove(filename);
        printf("Download interrupted.\n");
        return;
    }

    if (recv_line(sock, response, sizeof(response)) == 1)
        printf("%s\n", response);
    else
        printf("Connection lost after download.\n");
}

int main(void)
{
    int sock;
    struct sockaddr_in server_addr;
    char username[50];
    char password[50];
    char response[LINE_SIZE];
    char command[LINE_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        perror("socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    printf("[+] Connected to Concurrent FTP Server.\n");

    if (recv_line(sock, response, sizeof(response)) != 1)
    {
        printf("Server did not send a welcome message.\n");
        close(sock);
        return 1;
    }

    printf("%s\n", response);

    if (strncmp(response, "220 ", 4) != 0)
    {
        close(sock);
        return 1;
    }

    printf("USER ");
    fflush(stdout);

    if (fgets(username, sizeof(username), stdin) == NULL)
    {
        close(sock);
        return 1;
    }

    username[strcspn(username, "\n")] = '\0';

    if (username[0] == '\0' || strchr(username, ' ') != NULL ||
        strchr(username, '\r') != NULL)
    {
        printf("Invalid username.\n");
        close(sock);
        return 1;
    }

    char request[LINE_SIZE];

    int n = snprintf(request, sizeof(request),
                     "USER %s\n", username);

    if (n < 0 || (size_t)n >= sizeof(request) ||
        send_all(sock, request, strlen(request)) < 0)
    {
        close(sock);
        return 1;
    }

    if (recv_line(sock, response, sizeof(response)) != 1)
    {
        printf("Connection lost.\n");
        close(sock);
        return 1;
    }

    printf("%s\n", response);

    if (strncmp(response, "331 ", 4) != 0)
    {
        close(sock);
        return 1;
    }

    if (read_password(password, sizeof(password)) != 0)
    {
        close(sock);
        return 1;
    }

    if (password[0] == '\0' || strchr(password, ' ') != NULL ||
        strchr(password, '\r') != NULL)
    {
        memset(password, 0, sizeof(password));
        printf("Invalid password.\n");
        close(sock);
        return 1;
    }

    n = snprintf(request, sizeof(request),
                 "PASS %s\n", password);

    memset(password, 0, sizeof(password));

    if (n < 0 || (size_t)n >= sizeof(request) ||
        send_all(sock, request, strlen(request)) < 0)
    {
        close(sock);
        return 1;
    }

    if (recv_line(sock, response, sizeof(response)) != 1)
    {
        printf("Connection lost.\n");
        close(sock);
        return 1;
    }

    printf("%s\n", response);

    if (strncmp(response, "230 ", 4) != 0)
    {
        close(sock);
        return 1;
    }

    printf("\nCommands:\n");
    printf("DIR             - DIR server files\n");
    printf("STOR filename    - Upload a file\n");
    printf("RETR filename    - Download a file\n");
    printf("QUIT             - Disconnect\n\n");

    while (1)
    {
        printf("ftp> ");
        fflush(stdout);

        if (fgets(command, sizeof(command), stdin) == NULL)
            break;

        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "DIR") == 0)
        {
            DIR_files(sock);
        }
        else if (strncmp(command, "STOR ", 5) == 0)
        {
            upload_file(sock, command + 5);
        }
        else if (strncmp(command, "RETR ", 5) == 0)
        {
            download_file(sock, command + 5);
        }
        else if (strcmp(command, "QUIT") == 0)
        {
            if (send_all(sock, "QUIT\n", 5) == 0 &&
                recv_line(sock, response, sizeof(response)) == 1)
            {
                printf("%s\n", response);
            }

            break;
        }
        else
        {
            printf("Invalid command. Use DIR, STOR, RETR, or QUIT.\n");
        }
    }

    close(sock);
    return 0;
}

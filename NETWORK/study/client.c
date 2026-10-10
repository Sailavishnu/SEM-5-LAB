#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int sock, n;
    char msg[1024];
    struct sockaddr_in server;

    // Socket creation
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    // Server details
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Connection
    if (connect(sock, (struct sockaddr *)&server,
                sizeof(server)) < 0) {
        perror("Connection error");
        close(sock);
        return 1;
    }

    printf("Connected to server.\n");
    printf("Enter message: ");

    if (fgets(msg, sizeof(msg), stdin) == NULL) {
        printf("Input error.\n");
        close(sock);
        return 1;
    }

    // Send message
    if (send(sock, msg, strlen(msg), 0) < 0) {
        perror("Send error");
        close(sock);
        return 1;
    }

    // Receive echo
    n = recv(sock, msg, sizeof(msg) - 1, 0);
    if (n < 0) {
        perror("Receive error");
        close(sock);
        return 1;
    }
    if (n == 0) {
        printf("Server disconnected.\n");
        close(sock);
        return 1;
    }

    msg[n] = '\0';
    printf("Echo from server: %s", msg);

    close(sock);
    return 0;
}

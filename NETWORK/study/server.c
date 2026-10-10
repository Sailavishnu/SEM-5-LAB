#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int server_fd, client_fd, n;
    char msg[1024];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    // Socket creation
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket error");
        return 1;
    }

    // Server address
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(server_fd, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind error");
        close(server_fd);
        return 1;
    }

    // Listen
    if (listen(server_fd, 5) < 0) {
        perror("Listen error");
        close(server_fd);
        return 1;
    }

    printf("Server waiting for client...\n");

    // Accept
    client_fd = accept(server_fd, (struct sockaddr *)&client, &len);
    if (client_fd < 0) {
        perror("Accept error");
        close(server_fd);
        return 1;
    }

    printf("Client connected.\n");

    // Receive message
    n = recv(client_fd, msg, sizeof(msg) - 1, 0);
    if (n < 0) {
        perror("Receive error");
    }
    else if (n == 0) {
        printf("Client disconnected.\n");
    }
    else {
        msg[n] = '\0';
        printf("Message from client: %s", msg);

        // Send echo
        if (send(client_fd, msg, n, 0) < 0)
            perror("Send error");
        else
            printf("Echo sent successfully.\n");
    }

    close(client_fd);
    close(server_fd);
    return 0;
}

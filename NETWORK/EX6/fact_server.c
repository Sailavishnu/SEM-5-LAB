
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main() {
    int server, client, n, num, i;
    unsigned long long fact = 1;
    char msg[1024], response[1024];
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);

    server = socket(AF_INET, SOCK_STREAM, 0);
    if (server < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(8080);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(server, (struct sockaddr *)&server,
             sizeof(server)) < 0) {
        perror("Bind error");
        close(server);
        return 1;
    }

    if (listen(server, 5) < 0) {
        perror("Listen error");
        close(server);
        return 1;
    }

    printf("Server waiting for client...\n");

    client = accept(server, (struct sockaddr *)&client, &len);
    if (client < 0) {
        perror("Accept error");
        close(server);
        return 1;
    }

    n = recv(client, msg, sizeof(msg) - 1, 0);
    if (n <= 0){
        perror("Receive error");
        close(client);
        close(server);
        return 1;
    }

    msg[n] = '\0';
    num = atoi(msg);

    if (num < 0) {
        snprintf(response, sizeof(response),
                 "Factorial not defined for negative numbers");
    }
    else if (num > 20) {
        snprintf(response, sizeof(response),
                 "Number too large (maximum 20)");
    }
    else {
        for (i = 1; i <= num; i++)
            fact = fact * i;

        snprintf(response, sizeof(response),
                 "Factorial of %d = %llu", num, fact);
    }

    printf("%s\n", response);

    if (send(client, response, strlen(response), 0) < 0)
        perror("Send error");

    close(client);
    close(server);
    return 0;
}

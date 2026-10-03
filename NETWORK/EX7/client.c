#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define BUFSIZE 1024

/* ---------- helper functions ---------- */

void display_menu() {
    printf("\n===== STRING OPERATIONS (UDP) =====\n");
    printf("1. String concatenation\n");
    printf("2. String reverse\n");
    printf("3. Upper case to lower case\n");
    printf("4. Lower case to upper case\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
}

void read_string(const char *prompt, char *buf) {
    printf("%s", prompt);
    fgets(buf, BUFSIZE / 2, stdin);
    buf[strcspn(buf, "\n")] = '\0';
}

int create_client_socket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }
    return sockfd;
}

void send_request(int sockfd, struct sockaddr_in *server_addr, const char *request) {
    if (sendto(sockfd, request, strlen(request), 0,
               (struct sockaddr *)server_addr, sizeof(*server_addr)) < 0)
        perror("sendto");
}

void receive_response(int sockfd, char *response) {
    int n = recvfrom(sockfd, response, BUFSIZE - 1, 0, NULL, NULL);
    if (n < 0) { perror("recvfrom"); strcpy(response, "(no response)"); return; }
    response[n] = '\0';
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
    int sockfd = create_client_socket();
    struct sockaddr_in server_addr;
    char s1[BUFSIZE / 2], s2[BUFSIZE / 2], request[BUFSIZE * 2], response[BUFSIZE];
    const char *ip = (argc > 1) ? argv[1] : "127.0.0.1";
    int choice;

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "Invalid address\n"); exit(1);
    }

    while (1) {
        display_menu();
        if (scanf("%d", &choice) != 1) break;
        getchar();  /* consume newline */

        if (choice == 5) break;
        if (choice < 1 || choice > 4) { printf("Invalid choice.\n"); continue; }

        s2[0] = '\0';
        if (choice == 1) {
            read_string("Enter first string : ", s1);
            read_string("Enter second string: ", s2);
        } else {
            read_string("Enter string: ", s1);
        }

        snprintf(request, sizeof(request), "%d|%s|%s", choice, s1, s2);
        send_request(sockfd, &server_addr, request);
        receive_response(sockfd, response);
        printf("Result from server: %s\n", response);
    }

    close(sockfd);
    return 0;
}

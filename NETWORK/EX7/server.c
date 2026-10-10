#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8082
#define BUFSIZE 1024

/* ---------- string operation functions ---------- */

void concat_strings(const char *s1, const char *s2, char *result) {
    strcpy(result, s1);
    strcat(result, s2);
}

void reverse_string(const char *s, char *result) {
    int len = strlen(s);
    for (int i = 0; i < len; i++)
        result[i] = s[len - 1 - i];
    result[len] = '\0';
}

void to_lower(const char *s, char *result) {
    int i;
    for (i = 0; s[i] != '\0'; i++)
        result[i] = tolower((unsigned char)s[i]);
    result[i] = '\0';
}

void to_upper(const char *s, char *result) {
    int i;
    for (i = 0; s[i] != '\0'; i++)
        result[i] = toupper((unsigned char)s[i]);
    result[i] = '\0';
}

/* ---------- request handling ---------- */

/* Request format: choice|string1|string2 */
void process_request(const char *request, char *response) {
    char req[BUFSIZE], s1[BUFSIZE] = "", s2[BUFSIZE] = "";
    char *p1, *p2;
    int choice;

    strncpy(req, request, BUFSIZE - 1);
    req[BUFSIZE - 1] = '\0';

    choice = atoi(req);
    p1 = strchr(req, '|');
    if (p1) {
        p1++;
        p2 = strchr(p1, '|');
        if (p2) {
            *p2 = '\0';
            strcpy(s2, p2 + 1);
        }
        strcpy(s1, p1);
    }

    switch (choice) {
        case 1: concat_strings(s1, s2, response); break;
        case 2: reverse_string(s1, response);     break;
        case 3: to_lower(s1, response);           break;
        case 4: to_upper(s1, response);           break;
        default: strcpy(response, "Invalid option");
    }
}

/* ---------- socket setup ---------- */

int create_server_socket(int port) {
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(1);
    }
    return sockfd;
}

int main() {
    int sockfd = create_server_socket(PORT);
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char request[BUFSIZE], response[BUFSIZE];
    int n;

    printf("UDP string server listening on port %d...\n", PORT);

    while (1) {
        n = recvfrom(sockfd, request, BUFSIZE - 1, 0,
                     (struct sockaddr *)&client_addr, &addr_len);
        if (n < 0) { perror("recvfrom"); continue; }
        request[n] = '\0';
        printf("Request from %s: %s\n", inet_ntoa(client_addr.sin_addr), request);

        process_request(request, response);

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, addr_len);
    }

    close(sockfd);
    return 0;
}




























// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <ctype.h>
// #include <unistd.h>
// #include <arpa/inet.h>
// #include <sys/socket.h>

// int main() {
//     int sockfd, n, choice;
//     struct sockaddr_in server, client;
//     socklen_t len = sizeof(client);
//     char req[200], s1[100], s2[100], res[200];

//     sockfd = socket(AF_INET, SOCK_DGRAM, 0);

//     server.sin_family = AF_INET;
//     server.sin_addr.s_addr = INADDR_ANY;
//     server.sin_port = htons(8082);

//     bind(sockfd, (struct sockaddr *)&server, sizeof(server));

//     while (1) {
//         n = recvfrom(sockfd, req, 199, 0,
//                      (struct sockaddr *)&client, &len);
//         req[n] = '\0';

//         sscanf(req, "%d|%99[^|]|%99[^\n]",
//                &choice, s1, s2);

//         if (choice == 1)
//             sprintf(res, "%s%s", s1, s2);
//         else if (choice == 2) {
//             int i, l = strlen(s1);
//             for (i = 0; i < l; i++)
//                 res[i] = s1[l - 1 - i];
//             res[l] = '\0';
//         }
//         else if (choice == 3) {
//             int i;
//             for (i = 0; s1[i]; i++)
//                 res[i] = tolower(s1[i]);
//             res[i] = '\0';
//         }
//         else if (choice == 4) {
//             int i;
//             for (i = 0; s1[i]; i++)
//                 res[i] = toupper(s1[i]);
//             res[i] = '\0';
//         }
//         else
//             strcpy(res, "Invalid choice");

//         sendto(sockfd, res, strlen(res), 0,
//                (struct sockaddr *)&client, len);
//     }

//     close(sockfd);
//     return 0;
// }
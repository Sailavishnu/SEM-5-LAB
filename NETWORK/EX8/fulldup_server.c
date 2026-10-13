#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8082
#define SIZE 1024
#define MAX_QUEUE 10

typedef struct {
    struct sockaddr_in addr;
    socklen_t len;
    int valid;
} QueueItem;

int s;
struct sockaddr_in active_client;
socklen_t active_len;
int active = 0;
QueueItem queue[MAX_QUEUE] = {0};

int addr_equal(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_family == b->sin_family &&
            a->sin_addr.s_addr == b->sin_addr.s_addr &&
            a->sin_port == b->sin_port);
}

void add_to_queue(struct sockaddr_in *client, socklen_t len) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (!queue[i].valid) {
            queue[i].addr = *client;
            queue[i].len = len;
            queue[i].valid = 1;
            return;
        }
    }
}

int find_in_queue(struct sockaddr_in *client) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (queue[i].valid && addr_equal(&queue[i].addr, client))
            return i;
    }
    return -1;
}

int get_queue_count(void) {
    int count = 0;
    for (int i = 0; i < MAX_QUEUE; i++)
        if (queue[i].valid) count++;
    return count;
}

void remove_from_queue(int idx) {
    if (idx >= 0 && idx < MAX_QUEUE)
        queue[idx].valid = 0;
}

void promote_next(void) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (queue[i].valid) {
            active_client = queue[i].addr;
            active_len = queue[i].len;
            active = 1;
            queue[i].valid = 0;
            sendto(s, "CONNECTED", 9, 0,
                   (struct sockaddr *)&active_client, active_len);
            printf("First client disconnected. Now serving next client from queue.\n");
            printf("CLIENT CONNECTED\n");
            printf("Type EXIT to quit.\n\n");
            return;
        }
    }
    active = 0;
}

int main(void)
{
    char b[SIZE];
    struct sockaddr_in addr = {0}, from;
    socklen_t from_len;
    fd_set readfds;
    int maxfd, n, ret;

    s = socket(AF_INET, SOCK_DGRAM, 0);
    if (s < 0) {
        perror("socket");
        return 1;
    }

    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    printf("UDP SERVER WAITING ON PORT %d...\n", PORT);
    fflush(stdout);

    while (1) {
        /* Serve next queued client if any */
        if (!active && get_queue_count() > 0) {
            promote_next();
        }

        /* No active client and empty queue: block for HELLO */
        if (!active) {
            from_len = sizeof(from);
            n = recvfrom(s, b, SIZE - 1, 0,
                         (struct sockaddr *)&from, &from_len);
            if (n <= 0) continue;
            b[n] = '\0';

            if (strcmp(b, "HELLO") != 0) {
                printf("Invalid connection request\n");
                continue;
            }

            active_client = from;
            active_len = from_len;
            active = 1;
            sendto(s, "CONNECTED", 9, 0,
                   (struct sockaddr *)&active_client, active_len);
            printf("CLIENT CONNECTED\n");
            printf("Type EXIT to quit.\n\n");
        }

        /* Active chat session using select() */
        printf("Server> ");
        fflush(stdout);

        while (active) {
            FD_ZERO(&readfds);
            FD_SET(STDIN_FILENO, &readfds);
            FD_SET(s, &readfds);
            maxfd = (s > STDIN_FILENO) ? s : STDIN_FILENO;

            ret = select(maxfd + 1, &readfds, NULL, NULL, NULL);
            if (ret < 0) {
                perror("select");
                active = 0;
                break;
            }

            if (FD_ISSET(s, &readfds)) {
                from_len = sizeof(from);
                n = recvfrom(s, b, SIZE - 1, 0,
                             (struct sockaddr *)&from, &from_len);
                if (n <= 0) continue;
                b[n] = '\0';

                /* Message from someone other than active client */
                if (!addr_equal(&from, &active_client)) {
                    if (strcmp(b, "HELLO") == 0) {
                        if (find_in_queue(&from) == -1) {
                            if (get_queue_count() < MAX_QUEUE) {
                                add_to_queue(&from, from_len);
                                printf("\r\033[KNew client wants to connect. Added to queue. Queue count: %d\n", get_queue_count());
                                printf("Server> ");
                                fflush(stdout);
                                /* Send WAIT so client knows it is queued */
                                sendto(s, "WAIT", 4, 0,
                                       (struct sockaddr *)&from, from_len);
                            } else {
                                sendto(s, "QUEUE_FULL", 10, 0,
                                       (struct sockaddr *)&from, from_len);
                            }
                        }
                    }
                    continue;
                }

                /* Message from active client */
                printf("\r\033[K");
                if (strcmp(b, "EXIT") == 0) {
                    printf("Client left the chat.\n");
                    active = 0;
                    break;
                }

                printf("Client: %s\nServer> ", b);
                fflush(stdout);
            }

            if (FD_ISSET(STDIN_FILENO, &readfds)) {
                if (!fgets(b, SIZE, stdin)) {
                    active = 0;
                    break;
                }
                b[strcspn(b, "\n")] = '\0';
                if (b[0] == '\0') {
                    printf("Server> ");
                    fflush(stdout);
                    continue;
                }

                if (sendto(s, b, strlen(b), 0,
                           (struct sockaddr *)&active_client, active_len) < 0) {
                    perror("sendto");
                    active = 0;
                    break;
                }

                if (strcmp(b, "EXIT") == 0) {
                    printf("You left the chat.\n");
                    active = 0;
                    break;
                }

                printf("Server> ");
                fflush(stdout);
            }
        }
    }

    close(s);
    return 0;
}

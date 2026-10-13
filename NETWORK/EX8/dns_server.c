#include <stdio.h>
#include <string.h>
#include <unistd.h>
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

typedef struct {
    struct sockaddr_in addr;
    socklen_t len;
    int valid;
} ActiveClient;

int addr_equal(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_family == b->sin_family &&
            a->sin_addr.s_addr == b->sin_addr.s_addr &&
            a->sin_port == b->sin_port);
}

void lookup_ip(const char *domain, char *ip) {
    char *names[] = {"google.com", "yahoo.com", "example.com", "github.com", "facebook.com", "twitter.com"};
    char *ips[] = {"142.250.0.1", "98.137.11.163", "93.184.216.34", "140.82.121.4", "157.240.0.35", "104.244.42.1"};

    strcpy(ip, "Domain not found");
    for (int i = 0; i < 6; i++) {
        if (strcmp(domain, names[i]) == 0) {
            strcpy(ip, ips[i]);
            return;
        }
    }
}

int find_in_queue(QueueItem *queue, struct sockaddr_in *client) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (queue[i].valid && addr_equal(&queue[i].addr, client)) {
            return i;
        }
    }
    return -1;
}

void add_to_queue(QueueItem *queue, struct sockaddr_in *client, socklen_t len) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (!queue[i].valid) {
            queue[i].addr = *client;
            queue[i].len = len;
            queue[i].valid = 1;
            return;
        }
    }
}

void remove_from_queue(QueueItem *queue, int idx) {
    if (idx >= 0 && idx < MAX_QUEUE) {
        queue[idx].valid = 0;
    }
}

int get_queue_count(QueueItem *queue) {
    int count = 0;
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (queue[i].valid) count++;
    }
    return count;
}

void promote_next(int sockfd, QueueItem *queue, ActiveClient *active) {
    for (int i = 0; i < MAX_QUEUE; i++) {
        if (queue[i].valid) {
            active->addr = queue[i].addr;
            active->len = queue[i].len;
            active->valid = 1;
            queue[i].valid = 0;

            char *msg = "CONNECTED";
            sendto(sockfd, msg, strlen(msg), 0,
                   (struct sockaddr *)&active->addr, active->len);
            printf("First client disconnected. Now serving next client from queue.\n");
            return;
        }
    }
}

int main() {
    int sockfd, n;
    struct sockaddr_in server, client;
    socklen_t len = sizeof(client);
    char domain[SIZE], ip[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(sockfd, (struct sockaddr *)&server, sizeof(server));

    printf("DNS Server waiting...\n");

    ActiveClient active = {0};
    QueueItem queue[MAX_QUEUE] = {0};

    while (1) {
        n = recvfrom(sockfd, domain, SIZE - 1, 0,
                     (struct sockaddr *)&client, &len);
        if (n < 0) continue;

        domain[n] = '\0';

        /* ---- HANDLE CONNECT REQUEST ---- */
        if (strcmp(domain, "CONNECT") == 0) {
            if (!active.valid) {
                active.addr = client;
                active.len = len;
                active.valid = 1;
                char *msg = "CONNECTED";
                sendto(sockfd, msg, strlen(msg), 0,
                       (struct sockaddr *)&active.addr, active.len);
                printf("New client connected. Serving now.\n");
            } else if (addr_equal(&client, &active.addr)) {
                /* Already active, resend CONNECTED */
                char *msg = "CONNECTED";
                sendto(sockfd, msg, strlen(msg), 0,
                       (struct sockaddr *)&client, len);
            } else if (find_in_queue(queue, &client) == -1) {
                if (get_queue_count(queue) < MAX_QUEUE) {
                    add_to_queue(queue, &client, len);
                    char *msg = "WAIT: This is an iterative server, only one client can connect at a time. Please wait...";
                    sendto(sockfd, msg, strlen(msg), 0,
                           (struct sockaddr *)&client, len);
                    printf("New client wants to connect. Added to queue. Queue count: %d\n", get_queue_count(queue));
                } else {
                    char *msg = "QUEUE_FULL: Server busy. Queue full. Try again later.";
                    sendto(sockfd, msg, strlen(msg), 0,
                           (struct sockaddr *)&client, len);
                }
            }
            continue;
        }

        /* ---- HANDLE DISCONNECT (-1) ---- */
        if (strcmp(domain, "-1") == 0) {
            if (active.valid && addr_equal(&client, &active.addr)) {
                strcpy(ip, "Disconnected");
                sendto(sockfd, ip, strlen(ip), 0,
                       (struct sockaddr *)&client, len);
                printf("Active client disconnected.\n");
                active.valid = 0;

                if (get_queue_count(queue) > 0) {
                    promote_next(sockfd, queue, &active);
                }
            } else {
                int idx = find_in_queue(queue, &client);
                if (idx >= 0) {
                    remove_from_queue(queue, idx);
                    strcpy(ip, "Disconnected from queue");
                    sendto(sockfd, ip, strlen(ip), 0,
                           (struct sockaddr *)&client, len);
                    printf("Client removed from queue.\n");
                }
            }
            continue;
        }

        /* ---- HANDLE DOMAIN QUERY ---- */
        if (active.valid && addr_equal(&client, &active.addr)) {
            lookup_ip(domain, ip);
            printf("Request: %s, Response: %s\n", domain, ip);
            sendto(sockfd, ip, strlen(ip), 0,
                   (struct sockaddr *)&active.addr, active.len);
        } else {
            int idx = find_in_queue(queue, &client);
            if (idx >= 0) {
                char *msg = "WAIT: This is an iterative server, only one client can connect at a time. Please wait...";
                sendto(sockfd, msg, strlen(msg), 0,
                       (struct sockaddr *)&client, len);
            } else {
                char *msg = "ERROR: Please send CONNECT first.";
                sendto(sockfd, msg, strlen(msg), 0,
                       (struct sockaddr *)&client, len);
            }
        }
    }

    close(sockfd);
    return 0;
}

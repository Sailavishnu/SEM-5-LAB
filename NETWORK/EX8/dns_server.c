#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>

#define PORT 8082
#define SIZE 1024
#define MAX_QUEUE 10
#define MAX_ENTRIES 100

typedef struct {
    struct sockaddr_in addr;
    socklen_t len;
    int valid;
} QueueItem;

typedef struct {
    char domain[SIZE];
    char ip[SIZE];
} Entry;

int s;
struct sockaddr_in active_client;
socklen_t active_len;
int active = 0;
QueueItem queue[MAX_QUEUE] = {0};

Entry cache[MAX_ENTRIES];
int cache_count = 0;
Entry values[MAX_ENTRIES];
int values_count = 0;

int addr_equal(struct sockaddr_in *a, struct sockaddr_in *b) {
    return (a->sin_family == b->sin_family &&
            a->sin_addr.s_addr == b->sin_addr.s_addr &&
            a->sin_port == b->sin_port);
}

void load_file(const char *filename, Entry *entries, int *count) {
    FILE *fp = fopen(filename, "r");
    *count = 0;
    if (!fp) return;
    while (fscanf(fp, "%s %s", entries[*count].domain, entries[*count].ip) == 2) {
        (*count)++;
        if (*count >= MAX_ENTRIES) break;
    }
    fclose(fp);
}

void save_cache(const char *domain, const char *ip) {
    FILE *fp = fopen("cache.txt", "a");
    if (fp) {
        fprintf(fp, "%s %s\n", domain, ip);
        fclose(fp);
    }
}

void clear_cache_file(void) {
    FILE *fp = fopen("cache.txt", "w");
    if (fp) fclose(fp);
}

const char* find_in_entries(Entry *entries, int count, const char *domain) {
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].domain, domain) == 0)
            return entries[i].ip;
    }
    return NULL;
}

void lookup_ip(const char *domain, char *ip) {
    const char *result;

    result = find_in_entries(cache, cache_count, domain);
    if (result) {
        strcpy(ip, result);
        printf("Retrieved from cache: %s -> %s\n", domain, ip);
        return;
    }

    result = find_in_entries(values, values_count, domain);
    if (result) {
        strcpy(ip, result);
        printf("Retrieved from values: %s -> %s\n", domain, ip);
        strcpy(cache[cache_count].domain, domain);
        strcpy(cache[cache_count].ip, result);
        cache_count++;
        save_cache(domain, result);
        printf("Saved to cache: %s -> %s\n", domain, ip);
        return;
    }

    strcpy(ip, "Domain not found");
    printf("Domain not found: %s\n", domain);
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

    clear_cache_file();
    load_file("values.txt", values, &values_count);
    printf("Loaded %d entries from values\n", values_count);

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

    printf("DNS Server waiting on port %d...\n", PORT);
    fflush(stdout);

    while (1) {
        if (!active && get_queue_count() > 0) {
            promote_next();
        }

        if (!active) {
            from_len = sizeof(from);
            n = recvfrom(s, b, SIZE - 1, 0,
                         (struct sockaddr *)&from, &from_len);
            if (n <= 0) continue;
            b[n] = '\0';

            if (strcmp(b, "CONNECT") != 0) {
                printf("Invalid connection request\n");
                continue;
            }

            active_client = from;
            active_len = from_len;
            active = 1;
            sendto(s, "CONNECTED", 9, 0,
                   (struct sockaddr *)&active_client, active_len);
            printf("New client connected. Serving now.\n");
            printf("CLIENT CONNECTED\n");
        }

        while (active) {
            FD_ZERO(&readfds);
            FD_SET(s, &readfds);
            maxfd = s;

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

                if (!addr_equal(&from, &active_client)) {
                    if (strcmp(b, "CONNECT") == 0) {
                        if (find_in_queue(&from) == -1) {
                            if (get_queue_count() < MAX_QUEUE) {
                                add_to_queue(&from, from_len);
                                printf("New client wants to connect. Added to queue. Queue count: %d\n", get_queue_count());
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

                if (strcmp(b, "-1") == 0) {
                    char msg[] = "Disconnected";
                    sendto(s, msg, strlen(msg), 0,
                           (struct sockaddr *)&from, from_len);
                    printf("Active client disconnected.\n");
                    active = 0;
                    break;
                }

                char ip[SIZE];
                lookup_ip(b, ip);
                sendto(s, ip, strlen(ip), 0,
                       (struct sockaddr *)&active_client, active_len);
            }
        }
    }

    close(s);
    return 0;
}

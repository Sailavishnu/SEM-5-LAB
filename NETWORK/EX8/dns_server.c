
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

struct DNS {
    char domain[30], ip[20];
};

struct DNS dns[] = {
    {"google.com", "142.250.0.1"},
    {"yahoo.com", "98.137.11.163"},
    {"example.com", "93.184.216.34"},
    {"github.com", "140.82.121.4"},
    {"facebook.com", "157.240.0.35"},
    {"amazon.com", "205.251.242.54"}
};

int main()
{
    int s, n, i, active = 0;
    char msg[100], ip[100];
    struct sockaddr_in server = {0}, client, owner;
    socklen_t len;

    s = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(8082);
    server.sin_addr.s_addr = INADDR_ANY;

    if (bind(s, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("bind");
        close(s);
        return 1;
    }

    printf("DNS Server waiting on port 8082...\n");

    while (1) {
        len = sizeof(client);
        n = recvfrom(s, msg, 99, 0,
                     (struct sockaddr *)&client, &len);
        if (n <= 0) continue;
        msg[n] = '\0';

        int same = active &&
            client.sin_port == owner.sin_port &&
            client.sin_addr.s_addr == owner.sin_addr.s_addr;

        if (strcmp(msg, "CONNECT") == 0) {
            if (!active) {
                owner = client;
                active = 1;
                printf("Client connected\n");
            }

            same = client.sin_port == owner.sin_port &&
                   client.sin_addr.s_addr == owner.sin_addr.s_addr;

            char *reply = same ? "CONNECTED" : "WAIT";
            sendto(s, reply, strlen(reply), 0,
                   (struct sockaddr *)&client, len);
            continue;
        }

        if (!same) continue;

        if (strcmp(msg, "-1") == 0) {
            sendto(s, "DISCONNECTED", 12, 0,
                   (struct sockaddr *)&client, len);
            active = 0;
            printf("Client disconnected. Waiting for next client...\n");
            continue;
        }

        strcpy(ip, "Domain not found");

        for (i = 0; i < sizeof(dns) / sizeof(dns[0]); i++) {
            if (strcmp(msg, dns[i].domain) == 0) {
                strcpy(ip, dns[i].ip);
                break;
            }
        }

        sendto(s, ip, strlen(ip), 0,
               (struct sockaddr *)&client, len);
        printf("%s -> %s\n", msg, ip);
    }

    close(s);
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8080

static int sockfd = -1;

void handle_signal(int sig) {
    (void)sig;
    if (sockfd != -1)
        close(sockfd);
    exit(0);
}

int main() {
    char buffer[1024];
    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        return 1;
    }

    printf("UDP Server running on port %d...\n", PORT);

    while (1) {
        ssize_t n = recvfrom(sockfd, buffer, sizeof(buffer) - 1, 0,
                             (struct sockaddr*)&client_addr, &len);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }

        buffer[n] = '\0';
        printf("Received: %s\n", buffer);

        // Reply to client
        const char *reply = "Message received";
        if (sendto(sockfd, reply, strlen(reply), 0,
                   (struct sockaddr*)&client_addr, len) < 0)
            perror("sendto");
    }

    close(sockfd);
    return 0;
}
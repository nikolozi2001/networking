#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#define PORT 8080
#define BUFFER_SIZE 1024

static int sockfd = -1;

void handle_signal(int sig) {
    (void)sig;
    if (sockfd != -1)
        close(sockfd);
    exit(0);
}

int main() {
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t len = sizeof(server_addr);

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // Server config
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sockfd);
        return 1;
    }

    printf("UDP Client started...\n");

    while (1) {
        printf("Enter message: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL)
            break;

        // Send to server
        if (sendto(sockfd, buffer, strlen(buffer), 0,
                   (struct sockaddr*)&server_addr, len) < 0) {
            perror("sendto");
            continue;
        }

        // Receive response
        ssize_t n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0, NULL, NULL);
        if (n < 0) {
            perror("recvfrom");
            continue;
        }
        buffer[n] = '\0';

        printf("Server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}

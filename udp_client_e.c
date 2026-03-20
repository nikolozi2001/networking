#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr;
    socklen_t len = sizeof(server_addr);

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    // Server config
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    printf("✅ UDP Client started...\n");

    while (1) {
        printf("💬 Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        // Send to server
        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, len);

        // Receive response
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, NULL, NULL);
        buffer[n] = '\0';

        printf("📩 Server: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
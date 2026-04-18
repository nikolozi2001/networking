#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
//
#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 5
#define BROADCAST_ADDR "255.255.255.255"
//
int main() {
    // Socket შექმნა
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket შექმნა ვერ მოხერხდა");
        return 1;
    }
    // Broadcast ჩართვა
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST,
        &broadcast, sizeof(broadcast)) == -1) {
        perror("setsockopt შეცდომა");
        close(sockfd);
        return 2;
    }
    // Broadcast მისამართი
    struct sockaddr_in broadcast_addr;
    memset(&broadcast_addr, 0, sizeof(broadcast_addr));
    broadcast_addr.sin_family = AF_INET;
    broadcast_addr.sin_port   = htons(PORT);
    inet_pton(AF_INET, BROADCAST_ADDR, &broadcast_addr.sin_addr);
    // Broadcast გაგზავნა
    const char *message = "DISCOVER: ვინ არის სერვერი?";
    if (sendto(sockfd, message, strlen(message), 0,
        (struct sockaddr*)&broadcast_addr, sizeof(broadcast_addr)) == -1) {
        perror("sendto შეცდომა");
        close(sockfd);
        return 3;
    }
    printf("Broadcast გაიგზავნა -> %s:%d\n", BROADCAST_ADDR, PORT);
    // სერვერის პასუხის მოლოდინი
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    struct timeval timeout;
    timeout.tv_sec  = TIMEOUT_SEC;
    timeout.tv_usec = 0;
    //
    int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
    if (ret == -1) {
        perror("select შეცდომა");
        close(sockfd);
        return 4;
    }
    if (ret == 0) {
        printf("ტაიმაუტი: სერვერი არ გამოეხმაურა.\n");
        close(sockfd);
        return 5;
    }
    // პასუხის მიღება
    char buffer[BUFFER_SIZE];
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
        (struct sockaddr*)&server_addr, &server_len);
    if (n == -1) {
        perror("recvfrom შეცდომა");
        close(sockfd);
        return 6;
    }
    buffer[n] = '\0';
    printf("სერვერი [%s:%d]: %s\n",
    inet_ntoa(server_addr.sin_addr),
    ntohs(server_addr.sin_port), buffer);
    //
    close(sockfd);
    return 0;
}
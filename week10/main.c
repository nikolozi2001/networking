#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <errno.h>
#include <sys/select.h>
//
#define PORT 8080
#define BUFFER_SIZE 1024
#define TIMEOUT_SEC 10
//
volatile int running = 1;
//
void signal_handler(int sig) {
    running = 0;
}
//
int main() {
    signal(SIGINT,  signal_handler);
    signal(SIGTERM, signal_handler);
    // Socket შექმნა
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket შექმნა ვერ მოხერხდა");
        return 1;
    }
    // სერვერის მისამართი
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port        = htons(PORT);
    // Bind
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind ვერ მოხერხდა");
        close(sockfd);
        return 2;
    }
    printf("UDP სერვერი პორტზე %d გაეშვა...\n", PORT);
    printf("ტაიმაუტი: %d წამი | გასაჩერებლად: Ctrl+C\n", TIMEOUT_SEC);
    //
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    //
    while (running) {
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);
        struct timeval timeout;
        timeout.tv_sec  = TIMEOUT_SEC;
        timeout.tv_usec = 0;
        //
        int ret = select(sockfd + 1, &readfds, NULL, NULL, &timeout);
        if (ret == -1) {
            if (!running && errno == EINTR) break;
            perror("select შეცდომა");
            close(sockfd);
            return 3;
        }
        if (ret == 0) {
            printf("ტაიმაუტი: %d წამი გავიდა.\n", TIMEOUT_SEC);
            break;
        }
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
            (struct sockaddr*)&client_addr, &client_len);
        if (n == -1) {
            if (!running && errno == EINTR) break;
            perror("recvfrom შეცდომა");
            close(sockfd);
            return 4;
        }
        buffer[n] = '\0';
        printf("კლიენტი [%s:%d]: %s\n",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port), buffer);
        // Unicast პასუხი მხოლოდ გამგზავნს
        const char *response = "OFFER: მე ვარ სერვერი!";
        if (sendto(sockfd, response, strlen(response), 0,
            (struct sockaddr*)&client_addr, client_len) == -1) {
            perror("sendto შეცდომა");
            close(sockfd);
            return 5;
        }
        printf("Unicast პასუხი გაიგზავნა -> %s:%d\n",
        inet_ntoa(client_addr.sin_addr),
        ntohs(client_addr.sin_port));
    }
    //
    printf("\nსერვერი გაჩერდა.\n");
    close(sockfd);
    return 0;
}
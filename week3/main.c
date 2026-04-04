#include <stdio.h>
#include <unistd.h>
#include <sys/socket.h>

int main() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket() failed");
        return 1;
    }

    puts("Socket created successfully.\n");

    if (close(sockfd) == -1)
    {
        perror("close() failed");
        return 2;
    }

    puts("Socket closed successfully.\n");
    
    return 0;
}
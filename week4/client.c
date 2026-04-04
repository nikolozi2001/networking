#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket() failed");
        return errno;
    }

    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(8080);

    if (inet_pton(AF_INET, "127.0.0.1", &serveraddr.sin_addr) != 1)
    {
        perror("inet_pton() failed");
        close(sockfd);
        return errno;
    }

    if (connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("connect() failed");
        close(sockfd);
        return errno;
    }

    const char *message = "Hello, from client!";
    write(sockfd, message, strlen(message));

    char buffer[1024];
    int n = read(sockfd, buffer, sizeof(buffer) - 1);
    if (n >= 0)
    {
        buffer[n] = 0;
        printf("Server says: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
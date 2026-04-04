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
    {
        memset(&serveraddr, 0, sizeof(serveraddr));
        serveraddr.sin_family = AF_INET;
        serveraddr.sin_port = htons(8080);
        serveraddr.sin_addr.s_addr = INADDR_ANY;
    };

    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        perror("bind() failed");
        close(sockfd);
        return errno;
    }

    if (listen(sockfd, 5) == -1)
    {
        perror("listen() failed");
        close(sockfd);
        return errno;
    }

    printf("Server is listening on port 8080...\n");

    struct sockaddr_in cliaddr;
    socklen_t clilen = sizeof(cliaddr);
    int connfd = accept(sockfd, (struct sockaddr *)&cliaddr, &clilen);
    if (connfd == -1)
    {
        perror("accept() failed");
        close(sockfd);
        return errno;
    }

    printf("Client connected: %s:%d\n", 
        inet_ntoa(cliaddr.sin_addr), 
        ntohs(cliaddr.sin_port));

    char buffer[1024];
    ssize_t n = read(connfd, buffer, sizeof(buffer) - 1);
    if (n >= 0){
        buffer[n]= 0;
        printf("Received from client: %s\n", buffer);
    }

    const char *reply = "Hello from server!";
    write(connfd, reply, strlen(reply));

    close(connfd);
    close(sockfd);
    return 0;

}
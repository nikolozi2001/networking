#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>

#define PORT1 8888
#define PORT2 9999
#define BUF_SIZE 1024

static int create_udp_socket(int port) {
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { perror("socket"); exit(EXIT_FAILURE); }

    struct sockaddr_in addr = {
        .sin_family      = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port        = htons(port),
    };

    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("bind"); exit(EXIT_FAILURE);
    }
    return fd;
}

static void recv_and_print(int fd, int port) {
    char buf[BUF_SIZE] = {0};
    ssize_t n = recv(fd, buf, sizeof(buf) - 1, 0);
    if (n > 0)
        printf("[Port %d]: %s\n", port, buf);
}

int main(void) {
    int sock1 = create_udp_socket(PORT1);
    int sock2 = create_udp_socket(PORT2);
    int max_fd = (sock1 > sock2 ? sock1 : sock2);

    printf("სერვერი მუშაობს — პორტები %d, %d | გასასვლელად: exit\n", PORT1, PORT2);

    char buf[BUF_SIZE];
    fd_set readfds;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sock1, &readfds);
        FD_SET(sock2, &readfds);
        FD_SET(STDIN_FILENO, &readfds);

        struct timeval timeout = { .tv_sec = 5, .tv_usec = 0 };

        int ready = select(max_fd + 1, &readfds, NULL, NULL, &timeout);
        if (ready < 0) {
            perror("select");
            break;
        }

        if (ready == 0) {
            puts("[Timeout] 5 წამი სიჩუმე...");
            continue;
        }

        if (FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(buf, sizeof(buf), stdin)) break;
            buf[strcspn(buf, "\n")] = '\0';
            printf("[stdin]: %s\n", buf);
            if (strcmp(buf, "exit") == 0) break;
        }

        if (FD_ISSET(sock1, &readfds)) recv_and_print(sock1, PORT1);
        if (FD_ISSET(sock2, &readfds)) recv_and_print(sock2, PORT2);
    }

    close(sock1);
    close(sock2);
    return 0;
}

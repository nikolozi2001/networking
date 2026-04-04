#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <net/if_dl.h>
#include <arpa/inet.h>

void print_os_info() {
    struct utsname sys;

    if (uname(&sys) == 0) {
        printf("🖥 OS: %s\n", sys.sysname);
        printf("📦 Release: %s\n", sys.release);
        printf("🏷 Version: %s\n", sys.version);
        printf("💻 Machine: %s\n", sys.machine);
    }
}

void print_hostname() {
    char hostname[256];
    gethostname(hostname, sizeof(hostname));
    printf("🌐 Hostname: %s\n", hostname);
}

void print_network_info() {
    struct ifaddrs *ifaddr, *ifa;

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    printf("\n🌍 Network Interfaces:\n");

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr) continue;

        // IPv4
        if (ifa->ifa_addr->sa_family == AF_INET) {
            char ip[INET_ADDRSTRLEN];
            struct sockaddr_in *sa = (struct sockaddr_in *)ifa->ifa_addr;

            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
            printf("🔹 Interface: %s\n", ifa->ifa_name);
            printf("   IP Address: %s\n", ip);
        }

        // MAC Address
        if (ifa->ifa_addr->sa_family == AF_LINK) {
            struct sockaddr_dl *sdl = (struct sockaddr_dl *)ifa->ifa_addr;

            unsigned char *mac = (unsigned char *)LLADDR(sdl);
            if (sdl->sdl_alen == 6) {
                printf("🔸 Interface: %s\n", ifa->ifa_name);
                printf("   MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
                       mac[0], mac[1], mac[2],
                       mac[3], mac[4], mac[5]);
            }
        }
    }

    freeifaddrs(ifaddr);
}

int main() {
    printf("===== SYSTEM INFO =====\n\n");

    print_os_info();
    print_hostname();
    print_network_info();

    return 0;
}
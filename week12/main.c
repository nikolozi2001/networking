#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

void print_hex(unsigned char *p_data, unsigned int size)
{
    unsigned int index = 0;
    for (unsigned int i = 0; i < size / 16; i++)
    {
        for (unsigned int j = 0; j < 16; j++)
        {
            printf("%02x ", p_data[index]);
            index++;
        }
        puts("");
    }
    for (unsigned int i = 0; i < size % 16; i++)
    {
        printf("%02x ", p_data[index]);
        index++;
    }
    if (size % 16)
    {
        puts("");
    }
}

int main() {
    int fd = open("main", O_RDONLY);
    if (fd == -1)
    {
        perror("error open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        perror("error fstat");
        close(fd);
        return 1;
    }

    unsigned char *p_data = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (p_data == MAP_FAILED)
    {
        perror("error mmap");
        close(fd);
        return 1;
    }

    print_hex(p_data, st.st_size);

    munmap(p_data, st.st_size);
    close(fd);
    return 0;
}
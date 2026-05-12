//
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
//
#include "mef_format.h"
//
void print_hex(uint8_t* p_data, uint32_t size) {
    uint32_t index = 0;
    for(uint32_t i=0; i<size/16; i++) {
        for(uint32_t j=0; j<16; j++) {
            printf("%02x ", p_data[index]);
            index++;
        }
        puts("");
    }
    for(uint32_t i=0; i<size % 16; i++) {
        printf("%02x ", p_data[index]);
        index++;
    }
    if(size % 16) {
        puts("");
    }
}
//
int main(){
    //
    int fd = open("../00/mini64.mef", O_RDONLY);
    if(fd == -1){
        perror("error open()");
        return 1;
    }
    //
    struct stat stat;
    if(fstat(fd, &stat) == -1) {
        perror("error fstat()");
        close(fd);
        return 2;
    }
    //
    uint8_t *p_buff = malloc(stat.st_size);
    if(!p_buff) {
        fprintf(stderr, "error alloc heap\n");
        close(fd);
        return 3;
    }
    //
    if(read(fd, p_buff, stat.st_size) == -1) {
        perror("read()");
        free(p_buff);
        close(fd);
        return 2;
    }
    //
    pmef_hdr p_hdr = (pmef_hdr)p_buff;
    //
    if(p_hdr->magic != MEF_HDR_MAGIC) {
        fprintf(stderr, "error magic");
        free(p_buff);
        close(fd);
        return 3;
    }
    puts("magic:\t\tsuccess!");
    //
    switch(p_hdr->cpu_type) {
    case MEF_HDR_CPU_TYPE_X86:
        puts("cpu type:\tx86");
        break;
    case MEF_HDR_CPU_TYPE_X86_64:
        puts("cpu type:\tx86-64");
        break;
    case MEF_HDR_CPU_TYPE_AARCH:
        puts("cpu type:\tAarch 32");
        break;
    case MEF_HDR_CPU_TYPE_AARCH_64:
        puts("cpu type:\tAarch 64");
        break;
    case MEF_HDR_CPU_TYPE_UNKNOWN:
    default:
        fprintf(stderr, "error cpu unknown\n");
        free(p_buff);
        close(fd);
        return 4;
    }
    //
    switch(p_hdr->os_type) {
    case MEF_HDR_OS_TYPE_LINUX:
        puts("os type:\tLinux");
        break;
    case MEF_HDR_OS_TYPE_WINDOWS:
        puts("os type:\tWindows");
        break;
    case MEF_HDR_OS_TYPE_MAC_OS:
        puts("os type:\tMac OS");
        break;
    case MEF_HDR_OS_TYPE_UNKNOWN:
        fprintf(stderr, "error os unknown\n");
        free(p_buff);
        close(fd);
        return 5;
    }
    printf("section count:\t%u\n", p_hdr->sect_count);
    printf("code index:\t%u\n", p_hdr->code_index);
    //
    puts("\nsection headers");
    //
    psec_hdr p_sec_hdr = (psec_hdr)(p_buff + sizeof(mef_hdr));
    for(uint8_t i=0; i<p_hdr->sect_count; i++) {
        printf("file offset:\t%u\n", p_sec_hdr[i].file_offset);
        printf("file size:\t%u\n", p_sec_hdr[i].file_size);
        printf("memory offset:\t%u\n", p_sec_hdr[i].memory_offset);
        printf("memory size:\t%u\n", p_sec_hdr[i].memory_size);
        //
        printf("protect:\t");
        if(p_sec_hdr[i].protect & SEC_HDR_PROTECT_WRITEBLE) {
            printf("writable ");
        }
        //
        if(p_sec_hdr[i].protect & SEC_HDR_PROTECT_READABLE) {
            printf("readable ");
        }
        //
        if(p_sec_hdr[i].protect & SEC_HDR_PROTECT_EXECUTABLE) {
            printf("executable ");
        }
        //
        puts("");
        uint8_t *p_sect = (uint8_t*)(p_buff + p_sec_hdr[i].file_offset);
        print_hex(p_sect, p_sec_hdr[i].file_size);
        puts("");
    }
    //
    free(p_buff);
    close(fd);
    return 0;
}
//
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int func() {
    printf("Hello World!\n");
    return 0;
}

int main() {
    printf("1. A function call...\n");
    int ret = func();
    printf("2. Function complete, return: %x\n", ret);

    int fd = open("/proc/self/mem", O_RDWR);
    void* func_addr = &func;
    lseek(fd, (off_t) func_addr, SEEK_SET);
    write(fd, "\xB8\x42\x00\x00\x00\xC3", 6);
    close(fd);
    printf("3. A function call...\n");
    ret = func();
    printf("4. Function complete, return: %x\n", ret);
    return 0;
}
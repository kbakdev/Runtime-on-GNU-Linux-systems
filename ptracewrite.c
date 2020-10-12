#include <stdio.h>
#include <sys/ptrace.h>
#include <errno.h>
#include <stdint.h>
#include <sched.h>

int main() {
    pid_t pid = 911;
    uint8_t code[sizeof(long)] = { 0 };
    long* pcode = (long*) code;

    if(-1 == ptrace(PTRACE_ATTACH, pid, NULL, NULL)) {
        perror("ptrace attach");
        return 1;
    }

    errno = 0;
    *pcode = ptrace(PTRACE_PEEKTEXT, pid, 0x4005e1 -5, 0);
    if(errno != 0) {
        perror("ptrace peek");
        return 1;
    }

    code[0] = 0xEB;
    code[1] = 0xFE - 0x30 + 8; // result: 0xD6

    if(-1 == ptrace(PTRACE_POKETEXT, pid, 0x4005e1 - 5, *pcode)) {
        perror("ptrace poketext");
        return 1;
    }

    if(-1 == ptrace(PTRACE_DETACH, pid, NULL, NULL)) {
        perror("ptrace cont");
        return 1;
    }

    return 0;
}
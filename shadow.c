#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdbool.h>

typedef int (*func_connect)(int socket, const struct sockaddr *address, socklen_t address_len);

func_connect orig_connect;

#define MAKE_IP4(a,b,c,d) ((a) << 24 | (b) << 16 | (c) << 8 | (d))

bool load_orig_connect() {
    if(orig_connect != NULL)
        return true;

    void* libc = dlopen("/usr/lib64/libc.so.6", RTLD_LAZY);
    if(!libc) {
        printf("can't load libc\n");
        return false;
    }

    orig_connect = (func_connect) dlsym(libc, "connect");
    return orig_connect != NULL;
}

int connect(int socket, const struct sockaddr *address, socklen_t address_len)
{
    if(!load_orig_connect())
        return -EFAULT;

    if(address->sa_family != AF_INET && address_len != sizeof(struct sockaddr_in))
    {
        return orig_connect(socket, address, address_len);
    }

    struct sockaddr_in* sin = (struct sockaddr_in*) address;
    uint32_t addr = ntohl(sin->sin_addr.s_addr);

    if(addr == MAKE_IP4(212, 77, 98, 9)) {
        errno = ECONNREFUSED;
        return errno;
    }

    return orig_connect(socket,address, address_len);
}

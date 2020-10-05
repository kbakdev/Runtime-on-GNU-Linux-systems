#include <stdio.h>

static void init_func() __attribute__((constructor));
void init_func() {
    puts("Calling before main()!");
}

int main() {
    puts("Main() function.");
    return 0;
}
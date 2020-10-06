#include <stdio.h>

int var1 = 1;
int var2 = 2;

void putstr(int* ptr) {
    printf("val: %d\n", *ptr);
}

int main() {
    putstr(&var1);
    putstr(&var2);
}
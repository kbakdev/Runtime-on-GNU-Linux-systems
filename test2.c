#include <stdio.h>

char val;

void func(char* x) {
    printf("%d\n", *x);
}

int main() {
    fread(&val, 1, 1, stdin);
    func(&val);
    return 0;
}
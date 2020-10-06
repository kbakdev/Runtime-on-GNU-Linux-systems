#include <cstdio>

class Test {
public:
    Test() {
        puts("pre-main");
    }    
};

Test t;

int main() {
    puts("main");
    return 0;
}
#include "../include/Symbolic.h"

int main() {
    int a = 1;

    make_symbolic(&a, sizeof(a), "a");

    int i = 0;
    if (a + 2 == 100) {
        i += 2;
    } else {
        i += 3;
    }
    
    return 0;
}
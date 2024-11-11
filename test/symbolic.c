#include "../include/Symbolic.h"

int main() {
    int a = 1;

    make_symbolic(&a, sizeof(a), "a");

    int b = 2;
    int c = a + b;
    for (int i = 0; i < 5; i++) {
        a = a + 2;
    }
    return 0;
}
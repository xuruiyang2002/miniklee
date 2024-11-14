#include "../include/Symbolic.h"

int main() {
    int a = 1;

    make_symbolic(&a, sizeof(a), "a");

    int i = 0;
    if (a + 2 == 100) {
        // Should reach, a must be 98
        i += 2;
    } else {
        // Should reach, a can be assigned all values that are not 98
        i += 3;
    }
    
    return 0;
}
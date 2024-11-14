#include "../include/Symbolic.h"

int main() {
    int a = 1;

    make_symbolic(&a, sizeof(a), "a");

    int i = 0;
    if (4 - (3 - (2 - a)) == 2) {
        // Should reach, a must be 1
        i += 1;
    } else {
        // Should reach, a can be assigned all values that are not 1
        i += 3;
    }
    
    return 0;
}
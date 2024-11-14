#include "../include/Symbolic.h"

int main() {
    int a = 1;

    make_symbolic(&a, sizeof(a), "a");

    int i = 0;
    if (a == 3) {
        if (a + 2 == 100) {
            // Can not reach
            i += 1;
        } else {
            // Should reach, a = 3
            i += 2;
        }
    } else {
        if (a + 2 == 100) {
            // Should reach, a = 98
            i += 3;
        } else {
            // Should reach, a can be assigned all values that are not 2 and 98
            i += 4;
        }
    }
    
    return 0;
}
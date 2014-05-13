// Test error catching
#include <stdio.h>
#include <vector>

int main() {
    std::vector<int> x;

    x[1]=2;
    
    printf("x[1]=%d\n", x[1]);
}

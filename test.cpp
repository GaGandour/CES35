#include<stdio.h>

int main () {
    char buf = 1;

    printf("%d\n", buf);
    printf("%lu\n", sizeof(buf));
    printf("Hello World\n");
    return 0;
}

// 0x0110 = 0000 0001 0001 0000 = 256 + 16 = 272
// 0x1001 = 0001 0000 0000 0001 = 4096 + 1 = 4097
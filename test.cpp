#include<stdio.h>

int main () {
    char buf[2];
    buf[0] = 0x01;
    buf[1] = 0x10;
    
    int result = buf[1] << 8 | buf[0];

    printf("%d\n", result);

    printf("Hello World\n");
    return 0;
}

// 0x0110 = 0000 0001 0001 0000 = 256 + 16 = 272
// 0x1001 = 0001 0000 0000 0001 = 4096 + 1 = 4097
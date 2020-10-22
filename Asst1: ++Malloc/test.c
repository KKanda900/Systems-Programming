#include <stdio.h>

int main(int argc, char**argv){
    char bytes[4096];
    char* ptr = (char*)bytes;
    printf("%p\n", ptr+4097);
    return 0;
}
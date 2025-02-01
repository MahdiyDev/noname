#include <stdio.h>
#include <string.h>

struct Example {
    int id;
    char name[20];
    float value;
};

int main() {
    struct Example myStruct = {42, "Sample", 3.14};
    
    // Cast struct to char*
    char *bytePointer = (char *)&myStruct;

    // Access the struct as bytes
    for (size_t i = 0; i < sizeof(myStruct); i++) {
        printf("Byte %zu: 0x%02X\n", i, (unsigned char)bytePointer[i]);
    }

    return 0;
}

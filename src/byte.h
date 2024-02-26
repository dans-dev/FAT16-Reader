#ifndef BYTE_H
#define BYTE_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>


// Reads a given number of bytes with a given offset from a given file.
uint8_t* read_bytes(int fd, int offset, int bytes) {
    uint8_t* buffer = malloc(bytes); 
    lseek(fd, offset, SEEK_SET);
    read(fd, buffer, bytes);
    
    return buffer;
}

// Prints a number of bytes from a position in memory as hexadecimal values.
int print_bytes(uint8_t* buffer, int size) {
    for (int i = 0; i < size; i++) printf("0x%02X ",buffer[i]);
    printf("\n");

    return 0;
}


#endif
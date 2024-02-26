#ifndef FAT_H
#define FAT_H

#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "byte.h"
#include "structs.h"

// Reads the first FAT on the disk.
int read_fat(int fd, BootSector* bootptr, uint16_t* fat) {
    BootSector boot = *bootptr;
    int fat_start = (boot.BPB_RsvdSecCnt * boot.BPB_BytsPerSec);
    uint8_t* buffer = read_bytes(fd, fat_start, (boot.BPB_BytsPerSec  * boot.BPB_FATSz16));
    memcpy(fat, buffer, (boot.BPB_BytsPerSec  * boot.BPB_FATSz16));
    return 0;
}


// Returns the next cluster in a chain.
uint16_t next_cluster(uint16_t* fat, uint16_t cluster) {
    return fat[cluster];
}

#endif
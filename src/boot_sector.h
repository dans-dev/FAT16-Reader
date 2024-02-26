#ifndef BOOT_SECTOR_H
#define BOOT_SECTOR_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>

#include "byte.h"
#include "structs.h"


// Reads the Boot Sector of a FAT16 disk image into a BootSector struct specified, makes use of read_bytes().
int read_boot_sector(int fd, BootSector* boot) {
    uint8_t* buffer = read_bytes(fd,0,62);
    memcpy(boot, buffer, 62);
    return 0;
}


// Prints the Boot Sector information.
int print_boot_sector_info(BootSector* bootptr) {
    BootSector boot = *bootptr;
    printf("FAT16 Boot Sector Metadata\n");
    printf("Volume Label: "); for (int i = 0; i < 11; i++) printf("%c", boot.BS_VolLab[i]); printf("\n");
    printf("Bytes per Sector: %d\n", boot.BPB_BytsPerSec); 
    printf("Sectors per Cluster: %d\n", boot.BPB_SecPerClus);
    printf("Total Sectors (16): %d\n", boot.BPB_TotSec16); 
    printf("Total Sectors (32): %d\n", boot.BPB_TotSec32);
    printf("Reserved Sector Count: %d\n", boot.BPB_RsvdSecCnt);
    printf("Number of FATs: %d\n", boot.BPB_NumFATs); 
    printf("Sectors in FAT: %d\n", boot.BPB_FATSz16); 
    printf("Root Directory Size: %d\n", boot.BPB_RootEntCnt);

    return 0;
}

#endif
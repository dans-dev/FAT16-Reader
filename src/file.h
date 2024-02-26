#ifndef FILE_H
#define FILE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "structs.h"
#include "byte.h"

int open_file(Cursor* cursor, uint16_t cluster, int size) {
    File* f = (File*)malloc(sizeof(File));
    if (f == NULL) {
        return -1;
    }
    f->offset = 0;
    f->size = size;
    f->start_cluster = cluster;
    cursor->current_file = f;

    return 0;
}

int seek_file(Cursor* cursor, int offset, int mode) {
    switch (mode) {
        case 0:
            if (offset >= cursor->current_file->size) {
                printf("Error: Cannot seek outside of file!\n");
            } else {
                cursor->current_file->offset = offset;
            }
            break;
        case 1:
            if (offset + cursor->current_file->offset >= cursor->current_file->size) {
                printf("Error: Cannot seek outside of file!\n");
            } else {
                cursor->current_file->offset += offset;
            }
            break;
        default:
            break;
    }

    return 0;
}

int close_file(Cursor* cursor) {
    cursor->current_file = NULL;

    return 0;
}

int read_file(int fd, BootSector* boot_sector, uint16_t* fat, Cursor* cursor, int bytes, uint8_t* buffer) {
    if (cursor->current_file != NULL) {
        BootSector boot = *boot_sector;
        int cluster = cursor->current_file->start_cluster;
        int cluster_size = boot.BPB_BytsPerSec*boot.BPB_SecPerClus;
        int offset = cursor->current_file->offset;
        int position = (boot.BPB_RsvdSecCnt + (boot.BPB_NumFATs * boot.BPB_FATSz16) + (cluster-2)*boot.BPB_SecPerClus) * boot.BPB_BytsPerSec + boot.BPB_RootEntCnt*32;

        // move position in regards to offset,
        int cluster_offset = 0;
        while (offset / cluster_size > 1) {
            offset -= cluster_size;
            cluster_offset++;
        }

        for (int i = 0; i < cluster_offset; i++) {
            cluster = next_cluster(fat, cluster);
        }

        if (cluster < 65528) {
            int bytes_read = 0;
            for (int i = offset; bytes_read < bytes; i++) {
                if (i > cluster_size) {
                    cluster = next_cluster(fat, cluster);
                    i = 0;
                    position = (boot.BPB_RsvdSecCnt + (boot.BPB_NumFATs * boot.BPB_FATSz16) + (cluster-2)*boot.BPB_SecPerClus) * boot.BPB_BytsPerSec + boot.BPB_RootEntCnt*32;
                }
                uint8_t* byte = read_bytes(fd, position+i, 1);
                buffer[bytes_read] = *byte;
                bytes_read++;
                free(byte);
            }
        }
    } else {
        printf("Error Reading File: No File Open!\n");
    }
    return 0;
}

#endif
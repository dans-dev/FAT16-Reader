#ifndef STRUCTS_H
#define STRUCTS_H

#include <stdint.h>


// The structure of the Boot Sector
typedef struct __attribute__((__packed__)) {
    uint8_t BS_jmpBoot[ 3 ]; // x86 jump instr. to boot code
    uint8_t BS_OEMName[ 8 ]; // What created the filesystem
    uint16_t BPB_BytsPerSec; // Bytes per Sector
    uint8_t BPB_SecPerClus; // Sectors per Cluster
    uint16_t BPB_RsvdSecCnt; // Reserved Sector Count
    uint8_t BPB_NumFATs; // Number of copies of FAT
    uint16_t BPB_RootEntCnt; // FAT12/FAT16: size of root DIR
    uint16_t BPB_TotSec16; // Sectors, may be 0, see below
    uint8_t BPB_Media; // Media type, e.g. fixed
    uint16_t BPB_FATSz16; // Sectors in FAT (FAT12 or FAT16)
    uint16_t BPB_SecPerTrk; // Sectors per Track
    uint16_t BPB_NumHeads; // Number of heads in disk
    uint32_t BPB_HiddSec; // Hidden Sector count
    uint32_t BPB_TotSec32; // Sectors if BPB_TotSec16 == 0
    uint8_t BS_DrvNum; // 0 = floppy, 0x80 = hard disk
    uint8_t BS_Reserved1; // 
    uint8_t BS_BootSig; // Should = 0x29
    uint32_t BS_VolID; // 'Unique' ID for volume
    uint8_t BS_VolLab[ 11 ]; // Non zero terminated string
    uint8_t BS_FilSysType[ 8 ]; // e.g. 'FAT16 ' (Not 0 term.)
} BootSector;


// The structure of a Basic Directory Entry
typedef struct __attribute__((__packed__)) {
    uint8_t DIR_Name[ 11 ]; // Non zero terminated string
    uint8_t DIR_Attr; // File attributes
    uint8_t DIR_NTRes; // Used by Windows NT, ignore
    uint8_t DIR_CrtTimeTenth; // Tenths of sec. 0...199
    uint16_t DIR_CrtTime; // Creation Time in 2s intervals
    uint16_t DIR_CrtDate; // Date file created
    uint16_t DIR_LstAccDate; // Date of last read or write
    uint16_t DIR_FstClusHI; // Top 16 bits file's 1st cluster
    uint16_t DIR_WrtTime; // Time of last write
    uint16_t DIR_WrtDate; // Date of last write
    uint16_t DIR_FstClusLO; // Lower 16 bits file's 1st cluster
    uint32_t DIR_FileSize; // File size in bytes
} DirectoryEntry;


// The structure of a Long Directory Entry
typedef struct __attribute__((__packed__)) {
    uint8_t LDIR_Ord; // Order/ position in sequence/ set
    uint8_t LDIR_Name1[ 10 ]; // First 5 UNICODE characters
    uint8_t LDIR_Attr; // = ATTR_LONG_NAME (xx001111)
    uint8_t LDIR_Type; // Should = 0
    uint8_t LDIR_Chksum; // Checksum of short name
    uint8_t LDIR_Name2[ 12 ]; // Middle 6 UNICODE characters
    uint16_t LDIR_FstClusLO; // MUST be zero
    uint8_t LDIR_Name3[ 4 ]; // Last 2 UNICODE characters
} LongDirectoryEntry;


// The structure of a Long Directory Entry List (Linked)
typedef struct LongDirectoryEntryNode {
    LongDirectoryEntry entry;
    struct LongDirectoryEntryNode* next;
} LongDirectoryEntryNode;

typedef struct DirectoryEntryNode {
    DirectoryEntry entry;
    struct DirectoryEntryNode* next;
} DirectoryEntryNode;


// The structure used to represent a file
typedef struct File {
    uint16_t start_cluster;
    int offset;
    int size;
} File;


// The structure used to represent the cursor
typedef struct Cursor {
    uint16_t current_directory;
    File* current_file;
} Cursor;


// Splits a string by slashes.
char* get_word(const char *str, int index) {
    if (str == NULL) {
        return NULL;
    }

    char *temp = strdup(str);
    if (temp == NULL) {
        return NULL; 
    }

    const char delimiters[] = "/\\";
    char *token = strtok(temp, delimiters);
    int current_index = 0;

    while (token != NULL) {
        if (current_index == index) {
            char *result = strdup(token);
            free(temp);
            return result;
        }
        token = strtok(NULL, delimiters);
        current_index++;
    }
    free(temp);
    return NULL;
}

#endif
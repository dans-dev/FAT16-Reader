#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "structs.h"
#include "byte.h"
#include "fat.h"


// Function to add a long directory entry to the linked list
void add_long_entry(LongDirectoryEntryNode** head, LongDirectoryEntry entry) {
    LongDirectoryEntryNode* new_node = (LongDirectoryEntryNode*)malloc(sizeof(LongDirectoryEntryNode));
    new_node->entry = entry;
    new_node->next = *head;
    *head = new_node;
}


// Function to free the memory allocated for the linked list
void free_long_entries(LongDirectoryEntryNode* head) {
    LongDirectoryEntryNode* current = head;
    while (current != NULL) {
        LongDirectoryEntryNode* next = current->next;
        free(current);
        current = next;
    }
}


// Function to add a directory entry to the linked list
void add_entry(DirectoryEntryNode** head, DirectoryEntry entry) {
    DirectoryEntryNode* new_node = (DirectoryEntryNode*)malloc(sizeof(DirectoryEntryNode));
    new_node->entry = entry;
    new_node->next = NULL;

    // If the list is empty, new_node becomes the first node
    if (*head == NULL) {
        *head = new_node;
    } else {
        // Find the last node
        DirectoryEntryNode* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }

        // Append the new node
        current->next = new_node;
    }
}


// Function to free the memory allocated for the linked list
void free_entries(DirectoryEntryNode* head) {
    DirectoryEntryNode* current = head;
    while (current != NULL) {
        DirectoryEntryNode* next = current->next;
        free(current);
        current = next;
    }
}

// Reads the Root Directory of the disk
int read_root_directory(int fd, BootSector* boot_sector, DirectoryEntryNode** root_dir) {
    BootSector boot = *boot_sector;
    int root_dir_start = (boot.BPB_RsvdSecCnt + (boot.BPB_NumFATs * boot.BPB_FATSz16)) * boot.BPB_BytsPerSec;
    *root_dir = NULL; // Initialize the linked list head to NULL

    for (int i = 0; i < boot.BPB_RootEntCnt; i++) {
        uint8_t* buffer = read_bytes(fd, root_dir_start+(i*32), 32);
        DirectoryEntry entry;
        memcpy(&entry, buffer, 32);
        add_entry(root_dir, entry); // Add entry to the linked list
        free(buffer);
    }
    return 0;
}


// Reads a Directory of the disk
int read_directory(int fd, BootSector* boot_sector, uint16_t* fat, uint16_t cluster, DirectoryEntryNode** dir) {
    BootSector boot = *boot_sector;
    int dir_start = (boot.BPB_RsvdSecCnt + (boot.BPB_NumFATs * boot.BPB_FATSz16) + (cluster-2)*boot.BPB_SecPerClus) * boot.BPB_BytsPerSec + boot.BPB_RootEntCnt*32;
    
    *dir = NULL; // Initialize the linked list head to NULL

    uint8_t* buffer = read_bytes(fd, dir_start, 2048);
    uint8_t current_cluster[2048];
    memcpy(&current_cluster, buffer, 2048);
    int position = 0;

    while (cluster < 65528) {
        uint8_t entry_buffer[32];
        for (int i = 0; i < 32; i++) {
            entry_buffer[i] = current_cluster[position + i];
        }
        if (entry_buffer[0] != 0) {
            DirectoryEntry entry;
            memcpy(&entry, &entry_buffer, 32);
            add_entry(dir, entry); // Add entry to the linked list

            position += 32;

            if (position == 2048) {
                position = 0;
                cluster = next_cluster(fat, cluster);
                dir_start = (boot.BPB_RsvdSecCnt + (boot.BPB_NumFATs * boot.BPB_FATSz16) + (cluster - 2) * boot.BPB_SecPerClus) * boot.BPB_BytsPerSec + boot.BPB_RootEntCnt * 32;
                buffer = read_bytes(fd, dir_start, 2048);
                memcpy(&current_cluster, buffer, 2048);
                free(buffer); // Don't forget to free the allocated buffer
            }
        } else {
            cluster = 65528;
        }
    }

    return 0;
}


// Function to print the directory using linked list approach
int print_directory(DirectoryEntryNode* dir) {
    LongDirectoryEntryNode* long_entries = NULL;
    DirectoryEntryNode* current = dir;

    printf("Attributes   Last Modified          Cluster[0]    Size (B)      Name\n");
    while (current != NULL) {
        DirectoryEntry entry = current->entry;
        if (entry.DIR_Name[0] != 0) {
            uint8_t attr = entry.DIR_Attr;
            if (attr == 15) {
                add_long_entry(&long_entries, *((LongDirectoryEntry*)&entry));
            } else {
                // Attribute Handling
                int attributes[6] = {(attr & 32), (attr & 16), (attr & 8),  (attr & 4), (attr & 2), (attr & 1)};
                char attr_chars[6] = {  attributes[0] ? 'A' : '-',
                                        attributes[1] ? 'D' : '-',
                                        attributes[2] ? 'V' : '-',
                                        attributes[3] ? 'S' : '-',
                                        attributes[4] ? 'H' : '-',
                                        attributes[5] ? 'R' : '-' };

                // Date Handling
                int year = ((entry.DIR_WrtDate >> 9) & 127) + 1980;
                int month = (entry.DIR_WrtDate >> 5) & 15;
                int day = entry.DIR_WrtDate & 31;
                int hour = (entry.DIR_WrtTime >> 11) & 31;
                int minute = (entry.DIR_WrtTime >> 5) & 63;
                int second = (entry.DIR_WrtTime & 31) * 2;

                // Cluster Handling
                uint16_t start_cluster = (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;

                // Printing
                for (int j = 0; j < 6; j++) {
                    printf("%c", attr_chars[j]);
                }
                printf("       %02d-%02d-%04d %02d:%02d:%02d    ", day, month, year, hour, minute, second);
                printf("%-10d    ", start_cluster);
                printf("%-10d    ", entry.DIR_FileSize);

                if (long_entries == NULL) {
                    printf("%s", entry.DIR_Name);
                } else {

                    LongDirectoryEntryNode* c = long_entries;
                    while (c != NULL) {
                        for (int j = 0; j < 10; j += 2) {
                            if (c->entry.LDIR_Name1[j] != 255) {
                                printf("%c", c->entry.LDIR_Name1[j]);
                            }
                        }
                        for (int j = 0; j < 12; j += 2) {
                            if (c->entry.LDIR_Name2[j] != 255) {
                                printf("%c", c->entry.LDIR_Name2[j]);
                            }
                        }
                        for (int j = 0; j < 4; j += 2) {
                            if (c->entry.LDIR_Name3[j] != 255) {
                                printf("%c", c->entry.LDIR_Name3[j]);
                            }
                        }
                        LongDirectoryEntryNode* next = c->next;
                        free(c);
                        c = next;
                    }
                    long_entries = NULL;
                }
                printf("\n");
            }
            current = current->next;
        } else {
            break; // Exit the loop if the first byte of the entry is 0
        }
    }

    // Free the memory allocated for long entries
    free_long_entries(long_entries);

    return 0;
}


// Finds a directory entry based on name.
int find_directory_entry(DirectoryEntryNode* dir, char* path) {
    LongDirectoryEntryNode* long_entries = NULL;
    DirectoryEntryNode* current = dir;
    int entry_num = 0;
    int long_count = 0;

    while (current != NULL) {
        DirectoryEntry entry = current->entry;
        if (entry.DIR_Name[0] != 0) {
            uint8_t attr = entry.DIR_Attr;
            if (attr == 15) {
                add_long_entry(&long_entries, *((LongDirectoryEntry*)&entry));
                long_count++;
            } else {

                // Allowing short names to work too!
                int is_found = 1;
                for (int i = 0; i < 11; i++) {
                    if (path[i] != entry.DIR_Name[i]) {
                        is_found = 0;
                    }   
                }

                if (is_found) {
                    if (path[11] == 0) {
                        return entry_num;
                    }
                }

                // Handling long names.
                char name[11*long_count];
                if (long_entries == NULL) {
                    char* name = entry.DIR_Name;
                } else {
                    LongDirectoryEntryNode* c = long_entries;
                    int character = 0;
                    while (c != NULL) {
                        for (int j = 0; j < 10; j += 2) {
                            if (c->entry.LDIR_Name1[j] != 255) {
                                name[character] = c->entry.LDIR_Name1[j];
                                character++;
                            }
                        }
                        for (int j = 0; j < 12; j += 2) {
                            if (c->entry.LDIR_Name2[j] != 255) {
                                name[character] = c->entry.LDIR_Name2[j];
                                character++;
                            }
                        }
                        for (int j = 0; j < 4; j += 2) {
                            if (c->entry.LDIR_Name3[j] != 255) {
                                name[character] = c->entry.LDIR_Name3[j];
                                character++;
                            }
                        }
                        LongDirectoryEntryNode* next = c->next;
                        free(c);
                        c = next;
                    }
                    
                }
                long_entries = NULL;
                int long_count = 0;
                if (strcmp(path, name) == 0) {
                    return entry_num;
                }

            }

            // Checking . directories.
            if (strcmp(path, "..") == 0) {
                if (entry.DIR_Name[0] == '.' && entry.DIR_Name[1] == '.') {
                    return entry_num;
                }
            }

            if (strcmp(path, ".") == 0) {
                if (entry.DIR_Name[0] == '.' && entry.DIR_Name[1] != '.') {
                    return entry_num;
                }
            }
            current = current->next;
            entry_num++;
        } else {
            break; // Exit the loop if the first byte of the entry is 0
        }
    }

    // Free the memory allocated for long entries
    free_long_entries(long_entries);
    return -1;
}


// Returns the entry type based on an index.
int get_entry_type(DirectoryEntryNode* dir, int index) {
    DirectoryEntryNode* current = dir;
    int entry_num = 0;

    while (current != NULL) {
        DirectoryEntry entry = current->entry;
        if (entry_num == index) {
            uint8_t attr = entry.DIR_Attr;
            int attributes[6] = {(attr & 32), (attr & 16), (attr & 8),  (attr & 4), (attr & 2), (attr & 1)};
            if (attr == 15) return 3; // long
            if (attributes[0]) return 0; // file
            if (attributes[1]) return 1; // directory
            if (attributes[2]) return 2; // volume
        }
        entry_num++;
        current = current->next;
    }

    return -1;
}


// Returns a file start cluster based on an index.
int get_entry_start_cluster(DirectoryEntryNode* dir, int index) {
    DirectoryEntryNode* current = dir;
    int entry_num = 0;

    while (current != NULL) {
        DirectoryEntry entry = current->entry;
        if (entry_num == index) {
            return (entry.DIR_FstClusHI << 16) | entry.DIR_FstClusLO;
        }
        entry_num++;
        current = current->next;
        }

    return -1;
}


// Returns a file size based on an index.
int get_entry_size(DirectoryEntryNode* dir, int index) {
    DirectoryEntryNode* current = dir;
    int entry_num = 0;

    while (current != NULL) {
        DirectoryEntry entry = current->entry;
        if (entry_num == index) {
            return entry.DIR_FileSize;
        }
        entry_num++;
        current = current->next;
        }

    return -1;
}

#endif
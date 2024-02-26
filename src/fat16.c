#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#include "structs.h"
#include "byte.h"
#include "directory.h"
#include "fat.h"
#include "boot_sector.h"
#include "file.h"
#include "cursor.h"


char* CODE_VERSION = "1.0 Final";

// Main function
int main (int argc, char **argv) {
    printf("\nFAT16 Disk Image Viewer\n");
    printf("Version %s - Daniel Nicholson 2023\n\n", CODE_VERSION);
    if (argc == 2) {
        int fd = open(argv[1], 0);

        BootSector boot;
        printf("Reading Boot Sector         ");
        read_boot_sector(fd, &boot);
        printf("[DONE]\n");

        uint16_t fat[(boot.BPB_BytsPerSec / 2) * boot.BPB_FATSz16];
        printf("Reading FAT Table           ");
        read_fat(fd, &boot, fat);
        printf("[DONE]\n");

        DirectoryEntryNode* dir = NULL;
        printf("Reading Root Directory      ");
        read_root_directory(fd, &boot, &dir);
        printf("[DONE]\n");

        Cursor cursor;
        printf("Initializing Cursor         ");
        cursor.current_directory = 0;
        cursor.current_file = NULL;
        printf("[DONE]\n");

        printf("\nInitialization Complete! Type help for help.\n\n");

        int running = 1;
        while (running) {
            char* input = malloc(256);
            printf(">> ");
            scanf("%255s", input); // Memory Safety

            if (strcmp(input, "help") == 0) {
                printf("\nHelp Section:\n\n");
                printf("bootsectorinfo - Outputs the current Boot Sector info.\n");
                printf("ls - Lists the current directory.\n");
                printf("cd <path> - Moves the cursor to a given path.\n");
                printf("cat <path_to_file> - Outputs the entire file loaded.\n");
                printf("openfile <path_to_file> - Opens a file.\n");
                printf("closefile - Closes the current file.\n");
                printf("seekfile <offset> <mode> - Seeks within the file. MODES: 0 - SET, 1 - INCREMENT\n");
                printf("readfile <bytes> - Reads a given number of bytes from the open file.\n");
                printf("tracefat <cluster> - Traces a cluster chain.\n\n");

            } else if (strcmp(input, "bootsectorinfo") == 0) {
                printf("\n");
                print_boot_sector_info(&boot);
                printf("\n");

            } else if (strcmp(input, "ls") == 0) {
                printf("\n");
                print_directory(dir);
                printf("\n");

            } else if (strcmp(input, "cd") == 0) {
                char path[256];
                scanf("%[^\n]", path);
                
                move_cursor_to_path(fd, &boot, fat, &cursor, path + 1, 0, cursor.current_directory, 0);
                if (cursor.current_directory == 0) {
                    read_root_directory(fd, &boot, &dir);
                } else {
                    read_directory(fd, &boot, fat, cursor.current_directory, &dir);
                }

            } else if (strcmp(input, "cat") == 0) {
                char path[256];
                scanf("%[^\n]", path);
                cursor_open_file(fd, &boot, fat, &cursor, path + 1, 0, cursor.current_directory);
                if (cursor.current_file != NULL) {
                    printf("\n");
                    uint8_t* buffer = malloc(cursor.current_file->size);
                    int old_offset = cursor.current_file->offset;
                    cursor.current_file->offset = 0;
                    read_file(fd, &boot, fat, &cursor, cursor.current_file->size, buffer);
                    cursor.current_file->offset = old_offset;
                    for (int i = 0; i < cursor.current_file->size; i++) {
                        printf("%c",buffer[i]);
                    }
                    printf("\n\n");
                }

            } else if (strcmp(input, "openfile") == 0) {
                char path[256];
                scanf("%[^\n]", path);
                cursor_open_file(fd, &boot, fat, &cursor, path + 1, 0, cursor.current_directory);

            } else if (strcmp(input, "closefile") == 0) {
                close_file(&cursor);

            } else if (strcmp(input, "seekfile") == 0) {
                int seek_bytes = 0;
                int mode = -1;
                scanf("%d", &seek_bytes);
                scanf("%d", &mode);
                if (cursor.current_file != NULL) {
                    if (seek_bytes == 0 || mode == -1) {
                        printf("Error: Invalid Syntax! Type help for help.\n");
                    } else {
                        seek_file(&cursor, seek_bytes, mode);
                    }
                } else {
                    printf("Error: No File Open!\n");
                }

            } else if (strcmp(input, "readfile") == 0) {
                int read_bytes = 0;
                scanf("%d", &read_bytes);
                if (cursor.current_file != NULL) {
                    if (read_bytes == 0) {
                        printf("Error: Invalid Syntax! Type help for help.\n");
                    } else {
                        uint8_t* buffer = malloc(read_bytes);
                        read_file(fd, &boot, fat, &cursor, read_bytes, buffer);
                        for (int i = 0; i < read_bytes; i++) {
                            printf("%c",buffer[i]);
                        }
                    }
                    printf("\n\n");
                } else {
                    printf("Error: No File Open!\n");
                }

            } else if (strcmp(input, "tracefat") == 0) {
                int trace_cluster = 0;
                scanf("%d", &trace_cluster);
                if (trace_cluster == 0) {
                    printf("Error: Invalid Syntax! Type help for help.\n");
                } else {
                    printf("The cluster chain for %d is: ",trace_cluster);
                    while (trace_cluster < 65528) {
                        printf("%d ", trace_cluster);
                        trace_cluster = next_cluster(fat, trace_cluster);
                        if (trace_cluster < 65528) {
                            printf("-> ");
                        }
                    }
                    printf("\n\n");
                }

            } else {
                printf("Invalid command, type help for help.\n");
            }

            while (getchar() != '\n');  // Discard remaining characters in the input buffer
        }
    } else {
        printf("Invalid Arguments! Check syntax below:\n");
        printf("fat16 <filesystem> - <filesystem> is a valid FAT16 image\n");
    }
    return 0;
}

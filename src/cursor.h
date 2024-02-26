#ifndef CURSOR_H
#define CURSOR_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "structs.h"
#include "byte.h"
#include "directory.h"
#include "fat.h"
#include "file.h"


// Moves the cursor to the correct directory given a path.
int move_cursor_to_path(int fd, BootSector* boot_sector, uint16_t* fat, Cursor* cursor, char* path, int index, uint16_t old_position, int file_flag) { // file_flag as 1 means don't reset pos when at end
    char* path_section = get_word(path, index); // Gets the current word.

    // Checks to see if the path is null.
    if (path_section != NULL) {
        // Reads the directory currently selected.
        int current_dir = cursor->current_directory;
        DirectoryEntryNode* dir = NULL;

        if (current_dir == 0) {
            read_root_directory(fd, boot_sector, &dir);
        } else {
            read_directory(fd, boot_sector, fat, current_dir, &dir);
        }

        // Attempts to find the position of the entry requested in the path.
        int pos = find_directory_entry(dir, path_section);

        // If found, then move cursor, as well as calling function again.
        if (pos != -1) {
            int file_type = get_entry_type(dir, pos);
            if (file_type == 1) { // Checks to see if entry is a directory.
                cursor->current_directory = get_entry_start_cluster(dir, pos);
                index = move_cursor_to_path(fd, boot_sector, fat, cursor, path, index+1, old_position, file_flag); // Recursive call, moves to next path entry.
            } else {
                if (file_flag != 1) {
                    printf("Error: Invalid Path!\n");
                    cursor->current_directory = old_position;
                    return -1;
                }
            }
        } else { // If it cannot find the directory entry, set back to old directory position and throw error.
            printf("Error: Path Not Found!\n");
            cursor->current_directory = old_position;
            return -1;
        }
    }
    return index;
}


// Opens a file given by a path with the cursor. 
int cursor_open_file(int fd, BootSector* boot_sector, uint16_t* fat, Cursor* cursor, char* path, int index, uint16_t old_position) {
    char* path_section = get_word(path, index+1); // Gets the next word

    // Checks to see if it is the last entry in the path.
    if (path_section == NULL) {
        char* path_section = get_word(path, index); 
        if (path_section != NULL) {
            // Reads the directory currently selected.
            int current_dir = cursor->current_directory;
            DirectoryEntryNode* dir = NULL;

            if (current_dir == 0) {
                read_root_directory(fd, boot_sector, &dir);
            } else {
                read_directory(fd, boot_sector, fat, current_dir, &dir);
            }

            // Attempts to find the position of the entry requested in the path.
            int pos = find_directory_entry(dir, path_section);

            // If found, then move cursor, as well as calling function again.
            if (pos != -1) {
                int file_type = get_entry_type(dir, pos);
                if (file_type == 0) { // Checks to see if entry is a file.
                    // Opens the file/
                    open_file(cursor, get_entry_start_cluster(dir, pos), get_entry_size(dir, pos));
                    cursor->current_directory = old_position;
                } else {
                    printf("Error: Invalid File!\n");
                }
                
            } else {
                printf("Error: File Not Found! \n");
                cursor->current_directory = old_position;
            }
        } else {
            printf("Error: File Not Found! \n");
        }
    } else { // If entry not last part of path, move cursor first.
        int i = move_cursor_to_path(fd, boot_sector, fat, cursor, path, index, old_position, 1);
        if (i != -1) {
            cursor_open_file(fd, boot_sector, fat, cursor, path, i, old_position);
        }
    }

    return 0;
}

#endif 
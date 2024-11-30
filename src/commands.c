#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h> 
#include <stdlib.h>
#include "filesys.h"
#include "commands.h"

OpenFile open_files[MAX_OPEN_FILES];
int open_file_count = 0;

// Helper function to read a directory entry
int read_directory_entry(uint32_t cluster, int entry_index, char *name, uint8_t *attributes, uint32_t *next_cluster) {
    // Compute the sector where the cluster starts
    uint32_t sector = boot_sector.reserved_sector_count +
                      (boot_sector.num_fats * boot_sector.sectors_per_fat) +
                      ((cluster - 2) * boot_sector.sectors_per_cluster);

    // Compute the offset for the directory entry
    uint32_t offset = sector * boot_sector.bytes_per_sector + (entry_index * 32);

    fseek(image_file, offset, SEEK_SET);
    fread(name, 11, 1, image_file); // Read name (8+3 characters)
    fread(attributes, 1, 1, image_file); // Read attributes

    uint16_t cluster_low, cluster_high;
    fseek(image_file, offset + 20, SEEK_SET);
    fread(&cluster_high, sizeof(uint16_t), 1, image_file);
    fseek(image_file, offset + 26, SEEK_SET);
    fread(&cluster_low, sizeof(uint16_t), 1, image_file);
    *next_cluster = (cluster_high << 16) | cluster_low;

    // Null-terminate name and trim spaces
    name[11] = '\0';
    for (int i = 10; i >= 0 && name[i] == ' '; --i) {
        name[i] = '\0';
    }

    return 0;
}

void execute_cd(char *dirname) {
    char name[12];
    uint8_t attributes;
    uint32_t next_cluster;

    // Convert dirname to uppercase for case-insensitive comparison
    for (int i = 0; dirname[i]; i++) {
        dirname[i] = toupper(dirname[i]);
    }

    if (strcmp(dirname, ".") == 0) {
        return; // No change for current directory
    }

    if (strcmp(dirname, "..") == 0) {
        // Navigate to parent directory
        if (strcmp(current_path, "/") == 0) {
            fprintf(stderr, "Error: Already at root directory\n");
            return;
        }
        char *last_slash = strrchr(current_path, '/');
        if (last_slash) {
            *last_slash = '\0'; // Remove the last directory in path
        }
        if (strlen(current_path) == 0) {
            strcpy(current_path, "/"); // If empty, reset to root
        }
        current_cluster = boot_sector.root_cluster; // Reset cluster for simplicity
        return;
    }

    // Iterate through directory entries
    for (int i = 0; i < 16; ++i) { // Assuming max 16 entries in current directory
        read_directory_entry(current_cluster, i, name, &attributes, &next_cluster);
        
        // Convert entry name to uppercase for comparison
        for (int j = 0; name[j]; j++) {
            name[j] = toupper(name[j]);
        }

        if (strcmp(name, dirname) == 0) {
            if (attributes & 0x10) { // Directory
                current_cluster = next_cluster;

                // Update current_path without adding an extra slash
                if (strcmp(current_path, "/") == 0) {
                    strcat(current_path, dirname);
                } else {
                    strcat(current_path, "/");
                    strcat(current_path, dirname);
                }

                return;
            } else {
                fprintf(stderr, "Error: %s is not a directory\n", dirname);
                return;
            }
        }
    }

    fprintf(stderr, "Error: Directory %s not found\n", dirname);
}



void execute_ls() {
    char name[12];
    uint8_t attributes;
    uint32_t next_cluster;

    for (int i = 0; i < 16; ++i) { // Assuming max 16 entries in current directory
        read_directory_entry(current_cluster, i, name, &attributes, &next_cluster);
        if (name[0] != '\0' && (unsigned char)name[0] != 0xE5) { // Skip deleted/empty entries
            printf("%s%s\n", name, (attributes & 0x10) ? "/" : ""); // Add '/' for directories
        }
    }
}



void execute_info() {
    printf("Root Cluster Position: %u\n", boot_sector.root_cluster);
    printf("Bytes Per Sector: %u\n", boot_sector.bytes_per_sector);
    printf("Sectors Per Cluster: %u\n", boot_sector.sectors_per_cluster);
    printf("Total Clusters in Data Region: %u\n", boot_sector.total_sectors);
    printf("Entries in One FAT: %u\n", boot_sector.sectors_per_fat);
    printf("Size of Image (in bytes): %u\n",
           boot_sector.bytes_per_sector * boot_sector.sectors_per_cluster * boot_sector.total_sectors);
}

void execute_open(char *filename, char *flags) {
    if (open_file_count >= MAX_OPEN_FILES) {
        fprintf(stderr, "Error: Maxium open files reached\n");
        return;
    }

    if (strcmp(flags, "-r") != 0 && strcmp(flags, "-w") != 0 &&
        strcmp(flags, "-rw") != 0 && strcmp(flags, "-wr") != 0) {
        fprintf(stderr, "Error: Invalid mode %s\n", flags);
        return;
    }

    // Check if the file is already opened
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            fprintf(stderr, "Error: File %s is already open\n", filename);
            return;
        }
    }

    // Check if the file exists (you can use `fopen` in read mode to check existence)
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "Error: File %s does not exist\n", filename);
        return;
    }
    fclose(file);

    // Add the file to the open files list
    strncpy(open_files[open_file_count].filename, filename, sizeof(open_files[0].filename));
    strncpy(open_files[open_file_count].mode, flags, sizeof(open_files[0].mode));
    open_files[open_file_count].offset = 0;
    open_file_count++;
}

void execute_close(char *filename) {
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            // Remove the file from the open files list
            for (int j = i; j < open_file_count - 1; ++j) {
                open_files[j] = open_files[j + 1];
            }
            open_file_count--;
            return;
        }
    }
    fprintf(stderr, "Error: File %s is not open\n", filename);
}

void execute_lsof() {
    if (open_file_count == 0) {
        printf("No files are currently open.\n");
        return;
    }

    printf("Index\tFilename\tMode\tOffset\n");
    for (int i = 0; i < open_file_count; ++i) {
        printf("%d\t%s\t%s\t%u\n", i, open_files[i].filename, open_files[i].mode, open_files[i].offset);
    }
}

void execute_lseek(char *filename, uint32_t offset) {
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            // Check if offset is valid
            FILE *file = fopen(filename, "rb");
            fseek(file, 0, SEEK_END);
            uint32_t filesize = ftell(file);
            fclose(file);

            if (offset > filesize) {
                fprintf(stderr, "Error: Offset exceeds file size\n");
                return;
            }

            open_files[i].offset = offset;
            return;
        }
    }
    fprintf(stderr, "Error: File %s is not open\n", filename);
}

void execute_read(char *filename, uint32_t size) {
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            if (strchr(open_files[i].mode, 'r') == NULL) {
                fprintf(stderr, "Error: File %s is not open for reading\n", filename);
                return;
            }

            FILE *file = fopen(filename, "rb");
            fseek(file, 0, SEEK_END);
            uint32_t filesize = ftell(file);
            fseek(file, open_files[i].offset, SEEK_SET);

            uint32_t to_read = size;
            if (open_files[i].offset + size > filesize) {
                to_read = filesize - open_files[i].offset;
            }

            char *buffer = malloc(to_read + 1);
            fread(buffer, 1, to_read, file);
            buffer[to_read] = '\0';
            printf("Read Data: %s\n", buffer);
            free(buffer);

            open_files[i].offset += to_read;
            fclose(file);
            return;
        }
    }
    fprintf(stderr, "Error: File %s is not open\n", filename);
}

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h> 
#include <stdlib.h>
#include "filesys.h"
#include "commands.h"

// Helper macro for minimum value
#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

// Function to get the next cluster in the FAT chain
uint32_t get_next_cluster(uint32_t cluster) {
    uint32_t fat_offset = boot_sector.reserved_sector_count * boot_sector.bytes_per_sector;
    uint32_t entry_offset = fat_offset + (cluster * 4);

    uint32_t next_cluster;
    fseek(image_file, entry_offset, SEEK_SET);
    fread(&next_cluster, sizeof(uint32_t), 1, image_file);

    return next_cluster & 0x0FFFFFFF; // Mask to 28 bits
}

// Function to convert a cluster number to a sector number
uint32_t cluster_to_sector(uint32_t cluster) {
    return boot_sector.reserved_sector_count +
           (boot_sector.num_fats * boot_sector.sectors_per_fat) +
           ((cluster - 2) * boot_sector.sectors_per_cluster);
}

OpenFile open_files[MAX_OPEN_FILES];
int open_file_count = 0;

// Helper function to read a directory entry
int read_directory_entry(uint32_t cluster, int entry_index, char *name, uint8_t *attributes, uint32_t *next_cluster, uint32_t *size) {
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

     // Read file size
    fseek(image_file, offset + 28, SEEK_SET);
    fread(size, sizeof(uint32_t), 1, image_file);

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
    uint32_t size;
    

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
        read_directory_entry(current_cluster, i, name, &attributes, &next_cluster, &size);
        
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
    uint32_t size;

    for (int i = 0; i < 16; ++i) { // Adjust as per directory size
        if (read_directory_entry(current_cluster, i, name, &attributes, &next_cluster, &size) != 0) {
            fprintf(stderr, "Error reading directory entry %d\n", i);
            continue;
        }

        // Skip deleted entries
        if (name[0] == '\0' || (unsigned char)name[0] == 0xE5) {
            continue;
        }

        printf("%s%s\n", name, (attributes & 0x10) ? "/" : ""); // Add '/' for directories
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

void read_file_data(uint32_t start_cluster, uint32_t offset, uint32_t size, char *buffer) {
    uint32_t cluster_size = boot_sector.sectors_per_cluster * boot_sector.bytes_per_sector;
    uint32_t cluster = start_cluster;

    // Adjust to the correct cluster and offset
    while (offset >= cluster_size) {
        cluster = get_next_cluster(cluster);
        offset -= cluster_size;
    }

    // Read data across clusters
    uint32_t bytes_read = 0;
    while (bytes_read < size) {
        uint32_t sector = cluster_to_sector(cluster);
        fseek(image_file, sector * boot_sector.bytes_per_sector + offset, SEEK_SET);

        uint32_t to_read = MIN(size - bytes_read, cluster_size - offset);
        fread(buffer + bytes_read, 1, to_read, image_file);

        bytes_read += to_read;
        offset = 0;  // Offset only applies to the first cluster
        cluster = get_next_cluster(cluster);
    }
}

int file_exists_in_fat32(const char *filename, uint8_t *attributes, uint32_t *start_cluster, uint32_t *size) {
    char name[12];
    uint32_t next_cluster;

    // Iterate over directory entries in the current cluster
    for (int i = 0; i < 16; ++i) {  // Adjust max entries as per directory size
        if (read_directory_entry(current_cluster, i, name, attributes, &next_cluster, size) != 0) {
            fprintf(stderr, "Error reading directory entry %d\n", i);
            return 0;
        }

        // Skip empty or deleted entries
        if (name[0] == '\0' || (unsigned char)name[0] == 0xE5) {
            continue;
        }

        // Compare with the target filename 
        for (int j = 0; name[j]; j++) {
            name[j] = toupper(name[j]);
        }
        char temp_filename[12];
        strncpy(temp_filename, filename, 12);
        for (int j = 0; temp_filename[j]; j++) {
            temp_filename[j] = toupper(temp_filename[j]);
        }

        if (strcmp(name, temp_filename) == 0) {
            *start_cluster = next_cluster; // Record the starting cluster
            return 1;  
        }
    }

    return 0;  
}

void execute_open(char *filename, char *flags) {
    if (open_file_count >= MAX_OPEN_FILES) {
        fprintf(stderr, "Error: Maximum open files reached\n");
        return;
    }

    if (strcmp(flags, "-r") != 0 && strcmp(flags, "-w") != 0 &&
        strcmp(flags, "-rw") != 0 && strcmp(flags, "-wr") != 0) {
        fprintf(stderr, "Error: Invalid mode %s\n", flags);
        return;
    }

    
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            fprintf(stderr, "Error: File %s is already open\n", filename);
            return;
        }
    }

    uint8_t attributes;
    uint32_t start_cluster, size;
    if (!file_exists_in_fat32(filename, &attributes, &start_cluster, &size)) {
        fprintf(stderr, "Error: File %s does not exist\n", filename);
        return;
    }
    if (attributes & 0x10) {
        fprintf(stderr, "Error: %s is a directory, not a file\n", filename);
        return;
    }
    strncpy(open_files[open_file_count].filename, filename, sizeof(open_files[0].filename));
    strncpy(open_files[open_file_count].mode, flags, sizeof(open_files[0].mode));
    open_files[open_file_count].offset = 0;
    open_files[open_file_count].start_cluster = start_cluster; // Store starting cluster
    open_files[open_file_count].size = size; // Store file size
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
        printf("%d\t%s\t\t%s\t%u\n", i, open_files[i].filename, open_files[i].mode, open_files[i].offset);
    }
}

void execute_lseek(char *filename, uint32_t offset) {
    OpenFile *file = NULL;

    // Locate the file in the open files list
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            file = &open_files[i];
            break;
        }
    }

    if (!file) {
        fprintf(stderr, "Error: File %s is not open\n", filename);
        return;
    }

    
    if (offset > file->size) {
        fprintf(stderr, "Error: Offset %u is beyond the file size %u\n", offset, file->size);
        return;
    }

    // Set the offset
    file->offset = offset;
    printf("Offset for file %s set to %u bytes\n", filename, offset);
}

void execute_read(char *filename, uint32_t size) {
    OpenFile *file = NULL;

    // Locate the file in the open files list
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            file = &open_files[i];
            break;
        }
    }

    if (!file) {
        fprintf(stderr, "Error: File %s is not open\n", filename);
        return;
    }

    
    if (!strchr(file->mode, 'r')) {
        fprintf(stderr, "Error: File %s is not open for reading\n", filename);
        return;
    }

    // Calculate remaining bytes in the file
    uint32_t remaining_bytes = file->size - file->offset;
    uint32_t bytes_to_read = (size > remaining_bytes) ? remaining_bytes : size;

    if (bytes_to_read == 0) {
        fprintf(stderr, "Error: End of file reached\n");
        return;
    }

    // Read the data
    char buffer[bytes_to_read + 1];
    read_file_data(file->start_cluster, file->offset, bytes_to_read, buffer);
    buffer[bytes_to_read] = '\0'; // Null-terminate for safety
    printf("Data read from %s: %s\n", filename, buffer);

    // Update the file's offset
    file->offset += bytes_to_read;
    
    /* Debug output
    printf("Debug: File %s, Offset updated to %u, Bytes read=%u\n",
           filename, file->offset, bytes_to_read);
        
        */
    printf("Offset updated to %u\n", file->offset);

    
}

int is_directory_empty(uint32_t cluster) {
    char name[12];
    uint8_t attributes;
    uint32_t next_cluster;
    uint32_t size;

    for (int i = 0; i < 16; ++i) {
        if (read_directory_entry(cluster, i, name, &attributes, &next_cluster, &size) != 0) {
            return 0; 
        }

        if (name[0] != '\0' && (unsigned char)name[0] != 0xE5) { // Skip deleted/empty entries
            // Skip "." and ".." entries
            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                return 0; 
            }
        }
    }
    return 1; // Directory is empty
}

void clear_fat_entry(uint32_t cluster) {
    uint32_t fat_offset = boot_sector.reserved_sector_count * boot_sector.bytes_per_sector;
    uint32_t entry_offset = fat_offset + (cluster * 4);

    uint32_t free_cluster = 0; // Mark cluster as free
    fseek(image_file, entry_offset, SEEK_SET);
    fwrite(&free_cluster, sizeof(uint32_t), 1, image_file);
}


void reclaim_clusters(uint32_t start_cluster) {
    uint32_t cluster = start_cluster;
    while (cluster < 0xFFFFFF8) { // End of cluster chain
        uint32_t next_cluster = get_next_cluster(cluster);
        clear_fat_entry(cluster); // Function to clear the FAT entry for a cluster
        cluster = next_cluster;
    }
}

void execute_rm(const char *filename) {
    char name[12];
    uint8_t attributes;
    uint32_t next_cluster;
    uint32_t size;

    printf("Debug: Attempting to delete file %s\n", filename);


    // Check if the file is open
    for (int i = 0; i < open_file_count; ++i) {
        if (strcmp(open_files[i].filename, filename) == 0) {
            fprintf(stderr, "Error: File %s is currently open and cannot be deleted\n", filename);
            return;
        }
    }

    
    for (int i = 0; i < 16; ++i) { // Adjust as per directory size
        if (read_directory_entry(current_cluster, i, name, &attributes, &next_cluster, &size) != 0) {
            fprintf(stderr, "Error reading directory entry %d\n", i);
            continue;
        }

        // Match filename (case-insensitive)
        char temp_filename[12];
        strncpy(temp_filename, filename, sizeof(temp_filename));
        temp_filename[sizeof(temp_filename) - 1] = '\0';

        for (int j = 0; name[j]; j++) {
            name[j] = toupper(name[j]);
        }
        for (int j = 0; temp_filename[j]; j++) {
            temp_filename[j] = toupper(temp_filename[j]);
        }

        if (strcmp(name, temp_filename) == 0) {
            // Ensure it's a file (not a directory)
            if (attributes & 0x10) {
                fprintf(stderr, "Error: %s is a directory, not a file\n", filename);
                return;
            }

            // Mark the directory entry as deleted
            uint32_t sector = boot_sector.reserved_sector_count +
                              (boot_sector.num_fats * boot_sector.sectors_per_fat) +
                              ((current_cluster - 2) * boot_sector.sectors_per_cluster);
            uint32_t entry_offset = sector * boot_sector.bytes_per_sector + (i * 32);

            printf("Debug: Deleting entry at offset %u\n", entry_offset);
            fseek(image_file, entry_offset, SEEK_SET);
            uint8_t delete_flag = 0xE5;
            fwrite(&delete_flag, sizeof(uint8_t), 1, image_file);

            // Free the cluster chain
            uint32_t cluster = next_cluster;
            while (cluster < 0x0FFFFFF8 && cluster >= 2) {
                uint32_t fat_offset = boot_sector.reserved_sector_count * boot_sector.bytes_per_sector;
                uint32_t fat_entry = fat_offset + (cluster * 4);
                printf("Debug: Freeing cluster %u\n", cluster);

                fseek(image_file, fat_entry, SEEK_SET);
                uint32_t zero = 0x00000000;
                fwrite(&zero, sizeof(uint32_t), 1, image_file);

                // Read the next cluster
                fseek(image_file, fat_entry, SEEK_SET);
                fread(&cluster, sizeof(uint32_t), 1, image_file);
            }

            printf("File %s successfully deleted\n", filename);

            // Reload directory after deletion
           // reload_directory();
            return;
        }
    }

    fprintf(stderr, "Error: File %s not found\n", filename);
}

void execute_rmdir(char *dirname) {
    uint8_t attributes;
    uint32_t start_cluster, size;

    // Check if the directory exists
    if (!file_exists_in_fat32(dirname, &attributes, &start_cluster, &size)) {
        fprintf(stderr, "Error: Directory %s does not exist\n", dirname);
        return;
    }

    // Check if it is a directory
    if (!(attributes & 0x10)) {
        fprintf(stderr, "Error: %s is not a directory\n", dirname);
        return;
    }

    // Check if the directory is empty
    if (!is_directory_empty(start_cluster)) {
        fprintf(stderr, "Error: Directory %s is not empty\n", dirname);
        return;
    }

    // Check if any file in the directory is open
    for (int i = 0; i < open_file_count; ++i) {
        if (strncmp(open_files[i].filename, dirname, strlen(dirname)) == 0) {
            fprintf(stderr, "Error: A file in directory %s is open\n", dirname);
            return;
        }
    }

    // Remove directory entry
    if (remove_directory_entry(current_cluster, dirname) != 0) {
        fprintf(stderr, "Error: Failed to remove directory entry for %s\n", dirname);
        return;
    }

    // Reclaim clusters used by the directory
    reclaim_clusters(start_cluster);

    printf("Directory %s deleted successfully\n", dirname);
}

int remove_directory_entry(uint32_t cluster, char *name) {
    char entry_name[12];
    uint8_t attributes;
    uint32_t next_cluster;
    uint32_t size;

    for (int i = 0; i < 16; ++i) {
        if (read_directory_entry(cluster, i, entry_name, &attributes, &next_cluster, &size) != 0) {
            return -1; // Error reading the directory
        }

        if (strcmp(entry_name, name) == 0) {
            uint32_t sector = cluster_to_sector(cluster);
            uint32_t offset = sector * boot_sector.bytes_per_sector + (i * 32);

            fseek(image_file, offset, SEEK_SET);
            uint8_t deleted_flag = 0xE5;
            fwrite(&deleted_flag, sizeof(uint8_t), 1, image_file); // Mark entry as deleted
            return 0; 
        }
    }
    return -1; 
}


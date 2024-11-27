#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h> 
#include "filesys.h"
#include "commands.h"

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

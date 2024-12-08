#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesys.h"
#include "commands.h"

// Global Variables
FAT32BootSector boot_sector;
FILE *image_file = NULL;
char image_name[256];
char current_path[256] = "/";       // Initialize to root directory
uint32_t current_cluster;
FileEntry fileSystem[MAX_FILES] = {0}; // Simulated file system
int fileCount = 0;

unsigned int FAT[1000] = {0}; // Define the FAT array

// Function to initialize the simulated FAT array
void initialize_fat() {
    FAT[0] = 0xFFFFFFFF; // Reserved entry
    FAT[1] = 0xFFFFFFFF; // Reserved entry
    // All other entries are zero (already initialized)
}

void initialize_file_system() {
    // Example files and directories for simulation
    strcpy(fileSystem[fileCount].name, "testfile");
    fileSystem[fileCount].attr = ATTR_ARCHIVE;
    fileSystem[fileCount].size = 100;
    fileSystem[fileCount].offset = 0;
    fileSystem[fileCount].open = false;
    fileSystem[fileCount].write = false;
    fileCount++;

    strcpy(fileSystem[fileCount].name, "testdir");
    fileSystem[fileCount].attr = ATTR_DIRECTORY;
    fileSystem[fileCount].size = 0;
    fileSystem[fileCount].offset = 0;
    fileSystem[fileCount].open = false;
    fileSystem[fileCount].write = false;
    fileCount++;
}

void cleanup() {
    if (image_file) {
        fclose(image_file);
        image_file = NULL;
    }
    printf("Resources cleaned up. Exiting program...\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [FAT32 IMAGE]\n", argv[0]);
        return 1;
    }

    // Open the image file in read/write mode
    image_file = fopen(argv[1], "rb+");
    if (image_file == NULL) {
        perror("Error opening file");
        return 1;
    }

     // Store image name for the prompt
    snprintf(image_name, sizeof(image_name), "./%s", argv[1]);


    // Parse the boot sector
    parse_boot_sector();

    // Start shell interface
    shell_loop();

    // Cleanup resources when shell exits
    cleanup();

    return 0;
}

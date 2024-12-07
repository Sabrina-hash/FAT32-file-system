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
unsigned int FAT[1000] = {0};       // Initialize FAT to zero (simulate empty FAT)

// Function to initialize the simulated FAT array
void initialize_fat() {
    FAT[0] = 0xFFFFFFFF; // Reserved entry
    FAT[1] = 0xFFFFFFFF; // Reserved entry
    // All other entries are zero (already initialized)
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [FAT32 IMAGE]\n", argv[0]);
        return 1;
    }

    // Open the image file
    image_file = fopen(argv[1], "rb");
    if (image_file == NULL) {
        perror("Error opening file");
        return 1;
    }

     // Initialize FAT array
    initialize_fat();
    
    // Store image name for the prompt
    strncpy(image_name, argv[1], sizeof(image_name) - 1);

    // Parse the boot sector
    parse_boot_sector();

    // Start shell interface
    shell_loop();

    return 0;
}

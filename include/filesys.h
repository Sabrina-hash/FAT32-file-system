#ifndef FILESYS_H
#define FILESYS_H

#include <stdint.h>
#include <stdio.h>

// FAT32 Boot Sector Structure
typedef struct {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t num_fats;
    uint32_t sectors_per_fat;
    uint32_t root_cluster;
    uint32_t total_sectors;
} FAT32BootSector;

// Global Variables
extern FAT32BootSector boot_sector;
extern FILE *image_file;
extern char image_name[256];
extern char current_path[256];      // Current working directory path
extern uint32_t current_cluster;   // Current directory's starting cluster

// Function Prototypes
void parse_boot_sector();
void shell_loop();
void clean_exit();

#endif // FILESYS_H

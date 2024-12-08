#ifndef FILESYS_H
#define FILESYS_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

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

typedef struct {
    char DIR_Name[11];
    unsigned char DIR_Attr;      // Attribute
    unsigned char Reserved[10];  // Reserved bytes
    unsigned short DIR_FirstClusterLow;
    unsigned short DIR_FirstClusterHigh;
    unsigned int DIR_FileSize;   // File size in bytes
} DirectoryEntry;

typedef struct {
    char filename[256];
    char mode[4];
    uint32_t offset;
    uint32_t size;
    uint32_t start_cluster;
} OpenFile;

#define MAX_OPEN_FILES 128
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

// Simulated file entry
typedef struct {
    char name[11];      // 8.3 format name
    unsigned char attr; // File or directory attributes
    unsigned int size;  // File size in bytes
    unsigned int offset; // Current file offset
    bool open;          // Whether the file is open
    bool write;         // Whether the file is opened for writing
} FileEntry;

#define MAX_FILES 10

// Global Variables
extern FAT32BootSector boot_sector;
extern FILE *image_file;
extern char image_name[256];
extern char current_path[256];      // Current working directory path
extern uint32_t current_cluster;   // Current directory's starting cluster
extern OpenFile open_files[MAX_OPEN_FILES];
extern int open_file_count;
extern unsigned int FAT[1000];
extern FileEntry fileSystem[MAX_FILES];
extern int fileCount;

// Function Prototypes
void initialize_fat();
void parse_boot_sector();
void shell_loop();
void clean_exit();
void cleanup();

#endif // FILESYS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// Constants for directory attributes
#define ATTR_DIRECTORY 0x10
#define ATTR_ARCHIVE 0x20

// Structure for directory entries (short-name format, 32 bytes)
typedef struct {
    char DIR_Name[11];    // Short name (8.3 format)
    unsigned char DIR_Attr; // Attribute
    unsigned char Reserved[10]; // Reserved bytes
    unsigned short DIR_FirstClusterLow; // Low word of cluster number
    unsigned short DIR_FirstClusterHigh; // High word of cluster number
    unsigned int DIR_FileSize; // File size in bytes
} DirectoryEntry;

// Simulated file allocation table (FAT)
unsigned int FAT[1000] = {0};

// Function prototypes
void create_directory(const char *dirname);
void create_file(const char *filename);
unsigned int allocate_cluster();

// Helper to simulate creating a directory entry
void create_directory_entry(const char *name, unsigned char attr, unsigned int cluster) {
    DirectoryEntry entry = {0};
    strncpy(entry.DIR_Name, name, 11); // Copy name
    entry.DIR_Attr = attr;            // Set attribute
    entry.DIR_FirstClusterLow = cluster & 0xFFFF;
    entry.DIR_FirstClusterHigh = (cluster >> 16) & 0xFFFF;
    entry.DIR_FileSize = 0;           // Default size is 0

    // For now, print out the created entry for debugging
    printf("Created directory entry: %s (Attr: 0x%X, Cluster: %u)\n", name, attr, cluster);
}

// Function to create a directory
void create_directory(const char *dirname) {
    // Check if name is valid
    if (strlen(dirname) > 11) {
        fprintf(stderr, "Error: Directory name too long.\n");
        return;
    }

    // Allocate a cluster for the new directory
    unsigned int cluster = allocate_cluster();
    if (cluster == 0) {
        fprintf(stderr, "Error: No free clusters available.\n");
        return;
    }

    // Update FAT to mark this as end-of-cluster (EOC)
    FAT[cluster] = 0xFFFFFFFF;

    // Create the directory entry
    create_directory_entry(dirname, ATTR_DIRECTORY, cluster);

    // Simulate dot (.) and dot-dot (..) entries
    create_directory_entry(".", ATTR_DIRECTORY, cluster);
    create_directory_entry("..", ATTR_DIRECTORY, 0); // Assuming root cluster is 0
}

// Function to create a file
void create_file(const char *filename) {
    // Check if name is valid
    if (strlen(filename) > 11) {
        fprintf(stderr, "Error: File name too long.\n");
        return;
    }

    // Create the file entry (no cluster allocated yet)
    create_directory_entry(filename, ATTR_ARCHIVE, 0);
}

// Helper function to allocate a cluster (returns 0 if no free cluster)
unsigned int allocate_cluster() {
    for (unsigned int i = 2; i < sizeof(FAT) / sizeof(FAT[0]); i++) {
        if (FAT[i] == 0) {
            return i; // Free cluster found
        }
    }
    return 0; // No free cluster
}

int main() {
    char command[256], argument[256];
    
    while (1) {
        printf("> ");
        if (scanf("%255s %255s", command, argument) != 2) {
            fprintf(stderr, "Invalid command format.\n");
            continue;
        }
        
        if (strcmp(command, "mkdir") == 0) {
            create_directory(argument);
        } else if (strcmp(command, "creat") == 0) {
            create_file(argument);
        } else {
            fprintf(stderr, "Unknown command: %s\n", command);
        }
    }
    return 0;
}
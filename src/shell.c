#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesys.h"
#include "commands.h"



void parse_boot_sector() {
    fseek(image_file, 11, SEEK_SET);
    fread(&boot_sector.bytes_per_sector, sizeof(uint16_t), 1, image_file);

    fread(&boot_sector.sectors_per_cluster, sizeof(uint8_t), 1, image_file);

    fread(&boot_sector.reserved_sector_count, sizeof(uint16_t), 1, image_file);

    uint8_t num_fats;
    fread(&num_fats, sizeof(uint8_t), 1, image_file);
    boot_sector.num_fats = num_fats;

    fseek(image_file, 36, SEEK_SET); // Offset for sectors per FAT
    fread(&boot_sector.sectors_per_fat, sizeof(uint32_t), 1, image_file);

    fseek(image_file, 44, SEEK_SET); // Offset for root cluster
    fread(&boot_sector.root_cluster, sizeof(uint32_t), 1, image_file);

    fseek(image_file, 32, SEEK_SET); // Offset for total sectors
    fread(&boot_sector.total_sectors, sizeof(uint32_t), 1, image_file);

    // Compute total clusters in data region
    uint32_t reserved_sectors = boot_sector.reserved_sector_count +
                                (boot_sector.num_fats * boot_sector.sectors_per_fat);
    uint32_t data_sectors = boot_sector.total_sectors - reserved_sectors;
    boot_sector.total_sectors = data_sectors / boot_sector.sectors_per_cluster;
    current_cluster = boot_sector.root_cluster;
}

void shell_loop() {
    char command[256];
    while (1) {
        printf("[%s]%s> ", image_name, current_path);
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0; // Strip newline

        if (strncmp(command, "cd ", 3) == 0) {
            execute_cd(command + 3); // Pass directory name to `cd`
        } else if (strcmp(command, "ls") == 0) {
            execute_ls(); // List directory contents
        } else if (strcmp(command, "info") == 0) {
            execute_info(); // Display boot sector info
        } else if (strcmp(command, "exit") == 0) {
            clean_exit();
            break;
        } else if (strncmp(command, "open ", 5) == 0) {
            char *args = command + 5;
            char *filename = strtok(args, " ");
            char *flags = strtok(NULL, " ");
            execute_open(filename, flags);
        } else if (strncmp(command, "close ", 6) == 0) {
            execute_close(command + 6);
        } else if (strcmp(command, "lsof") == 0) {
            execute_lsof();
        } else if (strncmp(command, "lseek ", 6) == 0) {
            char *args = command + 6;
            char *filename = strtok(args, " ");
            char *offset_str = strtok(NULL, " ");
            if (filename && offset_str) {
                execute_lseek(filename, atoi(offset_str));
            }
        } else if (strncmp(command, "read ", 5) == 0) {
            char *args = command + 5;
            char *filename = strtok(args, " ");
            char *size_str = strtok(NULL, " ");
            if (filename && size_str) {
                execute_read(filename, atoi(size_str));
            }
        } else if (strncmp(command, "rm ", 3) == 0) {
            execute_rm(command + 3); // Pass filename to `rm`
        } else if (strncmp(command, "rmdir ", 6) == 0) {
            execute_rmdir(command + 6); // Pass directory name to `rmdir`
        } else {
            fprintf(stderr, "Error: Invalid command\n");
        }
    }
}



void clean_exit() {
    if (image_file) fclose(image_file);
    printf("Exiting program...\n");
}

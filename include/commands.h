#ifndef COMMANDS_H
#define COMMANDS_H

void execute_info();
void execute_cd(char *dirname);
void execute_ls();
void read_file_data(uint32_t start_cluster, uint32_t offset, uint32_t size, char *buffer);
int file_exists_in_fat32(const char *filename, uint8_t *attributes, uint32_t *start_cluster, uint32_t *size);
void execute_open(char *filename, char *flags);
void execute_close(char *filename);
void execute_lsof();
void execute_lseek(char *filename, uint32_t offset);
void execute_read(char *filename, uint32_t size);
int is_directory_empty(uint32_t cluster);
void clear_fat_entry(uint32_t cluster);
void reclaim_clusters(uint32_t start_cluster);
void execute_rm(const char *filename);
void execute_rmdir(char *dirname);
int remove_directory_entry(uint32_t cluster, char *name);
void reload_directory();




#endif // COMMANDS_H

#ifndef COMMANDS_H
#define COMMANDS_H

void execute_info();
void execute_cd(char *dirname);
void execute_ls();
void execute_open(char *filename, char *flags);
void execute_close(char *filename);
void execute_lsof();
void execute_lseek(char *filename, uint32_t offset);
void execute_read(char *filename, uint32_t size);
void create_directory(const char *dirname);
void create_file(const char *filename);
#endif // COMMANDS_H

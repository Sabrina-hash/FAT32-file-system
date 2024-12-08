 # Project3-Group25
This project provide us with a comprehensive understanding of basic file-system design and implementation by working with the FAT32 file system. 

## Group Members
- **Sabrina Polynice**: sp21p@fsu.edu
- **Veronica White**: vmw20@fsu.edu
- **Kyla Henry**: kgh21e@fsu.edu


## Division of Labor

### Part 1: Mounting Image File 
- **Responsibilities**: The user will need to mount the image file through command line arguments. You should read the image file and implement the correct structure to store the format of FAT32 and how to navigate it. You should close out the program and return an error message if the file does not exist. The user will then be greeted with a standard shell (like project 1) that will accept user input. Parses the boot sector. Prints the field name and corresponding value for each entry, one per line. Safely closes the program and frees up any allocated resources.
- **Assigned to**: Sabrina

### Part 2: Navigation
- **Responsibilities**: Changes the current working directory to the directory name, and print an error id the directory name doesn't exist or isn't in the directory. Print the name filed for the directories and files within the current working directory ncluding the “.” and “..” directories. 
- **Assigned to**: Sabrina 

### Part 3: Create
- **Responsibilities**: Creates a new directory in the current working directory with the name, prints an error if a directory/ file called directory name already exists. creates a file in the current working directory with a size of 0 bytes and with a name of file name prints an error if directory/ file directory name already exists. 
- **Assigned to**: Kyla

### Part 4: Read
- **Responsibilities**: The following commands will need to be implemented:

  - **open [FILENAME] [FLAGS]**:  
    Opens a file named FILENAME in the current working directory. A file can only be read from or written to if it is opened first. You will need to maintain some data structure of opened files and add FILENAME to it when open is called.  

    FLAGS is a flag and is only valid if it is one of the following:
    - -r - read-only.
    - -w - write-only.
    - -rw - read and write.
    - -wr - write and read.

    Initialize the offset of the file at 0. This can be stored in the open file data structure along with other info.  
    Print an error if the file is already opened, if the file does not exist, or an invalid mode is used.

  - **close [FILENAME]**:  
    Closes a file FILENAME in the current working directory.  
    Needs to remove the file entry from the open file data structure.  
    Print an error if the file is not opened or if the file does not exist.

  - **lsof**:  
    Lists all opened files.  
    Needs to list the index, file name, mode, offset, and path for every opened file. If no files are opened, notify the user.

  - **size [FILENAME]**:  
    Prints the size of the file [FILENAME] in the current working directory in bytes.  
    Print an error if [FILENAME] does not exist or if it is a directory.  
    File does not need to be open.

  - **lseek [FILENAME] [OFFSET]**:  
    Set the offset (in bytes) of file FILENAME for further reading or writing.  
    Store the value of OFFSET (in memory) and relate it to the file FILENAME.  
    Print an error if the file is not opened or does not exist.  
    Print an error if OFFSET is larger than the size of the file.

  - **read [FILENAME] [SIZE]**:  
    Read the data from a file in the current working directory with the name FILENAME.  
    Start reading from the file’s stored offset and stop after reading SIZE bytes.  
    If the offset + SIZE is larger than the size of the file, just read until end of file.  
    Update the offset of the file to offset + SIZE (or to the size of the file if you reached the end of the file).  
    Print an error if FILENAME does not exist, if FILENAME is a directory, or if the file is not opened for reading.

- **Assigned to**: Veronica

### Part 5: Update
- **Responsibilities**: The following commands will need to be implemented:

  - **write [FILENAME] [STRING]**:  
    Writes to a file in the current working directory with the name FILENAME.  
    Start writing at the file’s offset and stop after writing STRING. STRING is enclosed in “”.  
    If offset + size of STRING is larger than the size of the file, you will need to extend the length of the file to at least hold the data being written.  
    Update the offset of the file to offset + size of STRING.  
    Print an error if FILENAME does not exist, if FILENAME is a directory, or if the file is not opened for writing.

  - **rename [FILENAME] [NEW_FILENAME]**:  
    Rename a file or directory from FILENAME to NEW_FILENAME.  
    Print an error if FILENAME does not exist.  
    Print an error if NEW_FILENAME already exists.  
    File must be closed.  
    You cannot rename special directories like “.” or “..”.

- **Assigned to**: Kyla

### Part 6: Delete
- **Responsibilities**: The following commands will need to be implemented:

  - **rm [FILENAME]**:  
    Deletes the file named FILENAME from the current working directory.  
    This means removing the entry in the directory as well as reclaiming the actual file data.  
    Print an error if FILENAME does not exist, if the file is a directory, or if it is opened.

  - **rmdir [DIRNAME]**:  
    Remove a directory by the name of DIRNAME from the current working directory.  
    This command alone can only be used on an empty directory (do not remove “.” and “..”).  
    Make sure to remove the entry from the current working directory and to remove the data DIRNAME points to.  
    Print an error if the DIRNAME does not exist, if DIRNAME is not a directory, if DIRNAME is not an empty directory, or if a file is opened in that directory.

- **Assigned to**: Sabrina

### Extra Credit
For those seeking additional challenges and opportunities to earn extra credit, the following options are available: Our extras credit is done and completed

- **rm -r [DIRNAME]**:  
  The -r option adds the ability to rm to delete a directory and its content recursively.

- **dump [FILENAME/DIRNAME]**:  
  Perform a hex dump of a directory or file.


## File Listing
root/
├── part1/
    ├──include/
        ├── commands.h
        ├── filesys.h
    ├── src/
        ├── commands.c
        ├──main.c
        ├──shell.c
    ├──fat32.img
    ├──makefile
    ├──Readme.md
├──part2/
        ├──include/
        ├── commands.h
        ├── filesys.h
    ├── src/
        ├── commands.c
        ├──main.c
        ├──shell.c
    ├──fat32.img
    ├──makefile
    ├──Readme.md
├──part3/
    ├──include/
        ├── commands.h
        ├── filesys.h
    ├── src/
        ├── commands.c
        ├──main.c
        ├──shell.c
    ├──fat32.img
    ├──makefile
    ├──Readme.md
├──part5/
    ├──include/
        ├── commands.h
        ├── filesys.h
    ├── src/
        ├── commands.c
        ├──main.c
        ├──shell.c
    ├──fat32.img
    ├──makefile
    ├──Readme.md
├──part6/
    ├──include/
        ├── commands.h
        ├── filesys.h
    ├── src/
        ├── commands.c
        ├──main.c
        ├──shell.c
    ├──fat32.img
    ├──makefile
    ├──Readme.md

## How to Compile & Execute

### Requirements
- **Compiler**:
- **Dependencies**:

### Compilation
For a C/C++ example:
make

This will build the executable in ...
### Execution

./filesys fat32.img

## Bugs
- **no bugs reported**

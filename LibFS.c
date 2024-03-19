#include "LibFS.h"          // Include header file for virtual file system
#include "LibDisk.h"        // Include header file for virtual disk
#include <stdio.h>          // Include standard input-output library for standard I/O operations
#include <stdlib.h>         // Include standard library for memory allocation and other utilities
#include <string.h>         // Include string library for string manipulation functions
#include <unistd.h>         // Include POSIX standard library for system calls and types
#include <math.h>           // Include math library for mathematical functions

// Define constants
#define BLOCK_SIZE 512      // Define block size for disk blocks
#define NO_OF_INODES 256    // Define number of inodes
#define DISK_BLOCKS 1024    // Define total number of disk blocks
#define NO_OF_FILEDESCRIPTORS 256   // Define number of file descriptors

// Function prototypes
int FS_Sync();
void disk_Set();
int FS_Boot(char *path);
int find_file_in_directory(char *filename);
void add_file_to_directory(char *filename, int inode_num);

// Data structures
struct inode {
    int inode_num;          // Inode number
    int type;               // Type of file
    int filesize;           // Size of file
    int pointer[30];        // Pointers to data blocks of file (-1 if no files assigned)
};

struct DirectoryEntry {
    char file_name[16];     // Filename (maximum length of 15 characters)
    int inode_num;          // Inode number of the file
};

struct super_block {
    /* Directory information */
    int firstDbOfDir;                           // First data block of directory
    int noOfDbUsedinDir;                        // Number of data blocks used in directory
    /* Data information */
    int startingInodesDb;                       // Starting data block index for inodes
    int total_Usable_DB_Used_for_inodes;        // Total usable data blocks used for inodes
    int starting_DB_Index;                      // Starting data block index for data
    int total_Usable_DB;                        // Total usable data blocks
    char inode_Bitmap[NO_OF_INODES];           // Bitmap for inodes
    char datablock_Bitmap[DISK_BLOCKS];        // Bitmap for data blocks
};

// Global variables
int free_Inode_list_count = NO_OF_INODES;       // Count of free inodes
int free_Db_list_count = DISK_BLOCKS;           // Count of free data blocks
struct inode inode_arr[NO_OF_INODES] = {0};     // Array of inodes
struct DirectoryEntry dir_struct_entry[NO_OF_INODES] = {0};   // Array of directory entries
struct super_block sup_block = {0};                   // Super block structure
char disk_name[100] = "";                            // Disk image file name
int directory_map[NO_OF_INODES] = {0};                // Array to store directory entries (filename -> inode number)
int free_Db_list[DISK_BLOCKS] = {0};                   // Array to store free data blocks
int free_Inode_list[NO_OF_INODES] = {0};              // Array to store free inodes
int fileDescriptor_map[NO_OF_FILEDESCRIPTORS][2] = {{0}}; // Array to store file descriptors (fd -> [Inode number, file pointer])
int free_FD_list[NO_OF_FILEDESCRIPTORS] = {0};        // Array to store free file descriptors
int osErrno = 0;
int openfile_count = 0;                          // Count of open files

// Function to synchronize file system
int FS_Sync() {
    printf("FS_Sync\n");

    FILE *fp = fopen(disk_name, "rb+");    // Open disk image file for reading and writing in binary mode
    if (fp == NULL) {
        perror("Error opening disk image file");
        exit(EXIT_FAILURE);
    }

    // Retrieve super block from virtual disk and store into global super_block structure
    char sup_block_buff[sizeof(sup_block)] = {0};
    fread(sup_block_buff, 1, sizeof(sup_block), fp);
    memcpy(&sup_block, sup_block_buff, sizeof(sup_block));

    // Retrieve directory structure block from virtual disk and store into global dir_entry structure
    fseek(fp, (sup_block.firstDbOfDir) * BLOCK_SIZE, SEEK_SET);
    char dir_buff[sizeof(dir_struct_entry)] = {0};
    fread(dir_buff, 1, sizeof(dir_struct_entry), fp);
    memcpy(dir_struct_entry, dir_buff, sizeof(dir_struct_entry));

    // Retrieve inode block from virtual disk and store into global inode structure
    fseek(fp, (sup_block.startingInodesDb) * BLOCK_SIZE, SEEK_SET);
    char inode_buff[sizeof(inode_arr)] = {0};
    fread(inode_buff, 1, sizeof(inode_arr), fp);
    memcpy(inode_arr, inode_buff, sizeof(inode_arr));

    // Storing all filenames into array
    for (int i = NO_OF_INODES - 1; i >= 0; --i) {
        if (i >= 0 && i < NO_OF_INODES) {
            if (sup_block.inode_Bitmap[i] == 1) {
                directory_map[dir_struct_entry[i].inode_num] = i;
            } else {
                free_Inode_list[i] = i;
            }
        }
    }

    // Populate free_Db_list
    for (int i = DISK_BLOCKS - 1; i >= sup_block.starting_DB_Index; --i) {
        if (i >= sup_block.starting_DB_Index && i < DISK_BLOCKS) {
            if (sup_block.datablock_Bitmap[i] == 0) {
                free_Db_list[i] = i;
            }
        }
    }

    printf("Disk is mounted now\n");

    fclose(fp); // Close the disk image file

    return 1;
}

void disk_Set() {
    FILE *fp;
    char buffer[BLOCK_SIZE];
    struct super_block sup_block = {0}; // Initialize to all zeros

    // Open disk image file for writing in binary mode
    fp = fopen(disk_name, "wb");
    if (fp == NULL) {
        perror("Error opening disk image file");
        exit(EXIT_FAILURE);
    }

    // Initialize buffer to 0
    memset(buffer, 0, BLOCK_SIZE);

    // Write 0 to all disk blocks
    for (int i = 0; i < DISK_BLOCKS; ++i) {
        fwrite(buffer, 1, BLOCK_SIZE, fp);
    }

    // Set bitmap for metadata blocks and data blocks
    for (int i = 0; i < sup_block.starting_DB_Index; ++i) {
        sup_block.datablock_Bitmap[i] = 1; // Metadata blocks
    }
    for (int i = sup_block.starting_DB_Index; i < DISK_BLOCKS; ++i) {
        sup_block.datablock_Bitmap[i] = 0; // Data blocks
    }
    for (int i = 0; i < NO_OF_INODES; ++i) {
        sup_block.inode_Bitmap[i] = 0; // Initialize inode bitmap
    }
    // Initialize inode pointers
    for (int i = 0; i < NO_OF_INODES; ++i) {
        for (int j = 0; j < 30; j++) {
            inode_arr[i].pointer[j] = -1;
        }
    }

    // Store super block into the beginning of virtual disk
    fseek(fp, 0, SEEK_SET);
    fwrite(&sup_block, 1, sizeof(struct super_block), fp);

    // Store directory structure after super block into virtual disk
    fseek(fp, (sup_block.firstDbOfDir) * BLOCK_SIZE, SEEK_SET);
    fwrite(dir_struct_entry, 1, sizeof(dir_struct_entry), fp);

    // Store inode blocks after super block into virtual disk
    fseek(fp, (sup_block.startingInodesDb) * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_arr, 1, sizeof(inode_arr), fp);

    fclose(fp); // Close the disk image file

    printf("Virtual Disk %s created successfully\n", disk_name);
}

int FS_Boot(char *path) {
    strcpy(disk_name, path);
    printf("disk name: %s\n", disk_name);
    printf("FS_Boot %s\n", disk_name);

    // Check if the disk image file already exists
    if (access(disk_name, F_OK) != -1) {
        printf("The Disk already exists\n");
    } else {
        // Disk image doesn't exist, initialize it
        if (Disk_Init() == -1) {
            printf("Disk_Init() failed\n");
            osErrno = E_GENERAL;
            return -1;
        }

        // Save disk image
        printf("Saving disk image as: %s\n", disk_name);
        if (Disk_Save(disk_name) == -1) {
            printf("Disk_Save() failed\n");
            osErrno = E_GENERAL;
            return -1;
        }
        printf("Disk image saved successfully\n");

        // Set up disk
        disk_Set();
    }

    return 0;
}

// Function to check if the file exists in the directory
int find_file_in_directory(char *filename) {
    for (int i = 0; i < NO_OF_INODES; i++) {
        if (strcmp(dir_struct_entry[i].file_name, filename) == 0) {
            return i; // Return inode number if file found
        }
    }
    return -1; // Return -1 if file not found
}

// Function to add a file to the directory
void add_file_to_directory(char *filename, int inode_num) {
    for (int i = 0; i < NO_OF_INODES; i++) {
        if (strlen(dir_struct_entry[i].file_name) == 0) { // Find empty slot in directory
            strcpy(dir_struct_entry[i].file_name, filename);
            dir_struct_entry[i].inode_num = inode_num;
            return;
        }
    }
}

/*********************************************************************
 File_Create function - it will create the blank file into disk      *
**********************************************************************/
// Function to create a file
int File_Create(char *filename) {
    printf("FS_Create\n");

    int nextin, nextdb;

    // Check if the file already exists
    if (find_file_in_directory(filename) != -1) {
        printf("Error: File '%s' already exists.\n", filename);
        return -1;
    }

    // Check if there are free inodes
    if (free_Inode_list_count == 0) {
        printf("Error: No more files can be created. Maximum number of files reached.\n");
        return -1;
    }

    // Check if there are free data blocks
    if (free_Db_list_count == 0) {
        printf("Error: Memory is full.\n");
        return -1;
    }

    // Get the next free inode and data block
    nextin = free_Inode_list[--free_Inode_list_count];
    nextdb = free_Db_list[--free_Db_list_count];

    // Update inode properties
    inode_arr[nextin].inode_num = nextin;
    inode_arr[nextin].filesize = 0;
    inode_arr[nextin].pointer[0] = nextdb;

    // Update directory entry
    add_file_to_directory(filename, nextin);

    printf("File '%s' created successfully.\n", filename);

    return 0;
}

/*********************************************************************
 File_Open - it will open the file                                   *
**********************************************************************/
int File_Open(char *filename) {
    printf("FS_Open\n");

    int inode_no = find_file_in_directory(filename);

    // Check if the file exists in the directory
    if (inode_no == -1) {
        printf("Error: File '%s' not found.\n", filename);
        return -1;
    }

    // Check if there are available file descriptors
    if (openfile_count == NO_OF_FILEDESCRIPTORS) {
        printf("Error: File descriptors are not available.\n");
        return -1;
    }

    // Get a free file descriptor
    int fd;
    for (fd = 0; fd < NO_OF_FILEDESCRIPTORS; fd++) {
        if (free_FD_list[fd] == 0) {
            free_FD_list[fd] = 1;
            break;
        }
    }

    // Update file descriptor map
    fileDescriptor_map[fd][0] = inode_no;
    fileDescriptor_map[fd][1] = 0;
    openfile_count++;

    printf("File '%s' opened successfully. File descriptor: %d\n", filename, fd);

    return fd;
}

int File_Read(int fd, void *buffer, int size) {
    printf("FS_Read\n");
    return 0;
}

int File_Write(int fd, void *buffer, int size) {
    printf("FS_Write\n");
    return 0;
}

int File_Seek(int fd, int offset) {
    printf("FS_Seek\n");
    return 0;
}

int File_Close(int fd) {
    printf("FS_Close\n");
    return 0;
}

int File_Unlink(char *file) {
    printf("FS_Unlink\n");
    return 0;
}

int Dir_Create(char *path) {
    printf("Dir_Create %s\n", path);
    return 0;
}

int Dir_Size(char *path) {
    printf("Dir_Size\n");
    return 0;
}

int Dir_Read(char *path, void *buffer, int size) {
    printf("Dir_Read\n");
    return 0;
}

int Dir_Unlink(char *path) {
    printf("Dir_Unlink\n");
    return 0;
}
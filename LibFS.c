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
int read_block(int block, char* buf);
int block_write(int block, char *buf, int size_to_wrt, int start_pos_to_wrt);
int FS_Sync();
void disk_Set();
int FS_Boot(char *path);

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
struct inode inode_arr[NO_OF_INODES];          // Array of inodes
struct DirectoryEntry dir_struct_entry[NO_OF_INODES];   // Array of directory entries
struct super_block sup_block;                   // Super block structure
char disk_name[100];                            // Disk image file name
char filename[16];                              // File name
int directory_map[NO_OF_INODES];                // Array to store directory entries (filename -> inode number)
int free_Db_list[DISK_BLOCKS];                   // Array to store free data blocks
int free_Inode_list[NO_OF_INODES];              // Array to store free inodes
int fileDescriptor_map[NO_OF_FILEDESCRIPTORS][2]; // Array to store file descriptors (fd -> [Inode number, file pointer])
int osErrno;

// Function to synchronize file system
int FS_Sync() {
    printf("FS_Sync\n");

    FILE *fp = fopen(disk_name, "rb+");    // Open disk image file for reading and writing in binary mode

    // Retrieve super block from virtual disk and store into global super_block structure
    char sup_block_buff[sizeof(sup_block)];
    memset(sup_block_buff, 0, sizeof(sup_block));
    fread(sup_block_buff, 1, sizeof(sup_block), fp);
    memcpy(&sup_block, sup_block_buff, sizeof(sup_block));

    // Retrieve directory structure block from virtual disk and store into global dir_entry structure
    fseek(fp, (sup_block.firstDbOfDir) * BLOCK_SIZE, SEEK_SET);
    char dir_buff[sizeof(dir_struct_entry)];
    memset(dir_buff, 0, sizeof(dir_struct_entry));
    fread(dir_buff, 1, sizeof(dir_struct_entry), fp);
    memcpy(dir_struct_entry, dir_buff, sizeof(dir_struct_entry));

    // Retrieve inode block from virtual disk and store into global inode structure
    fseek(fp, (sup_block.startingInodesDb) * BLOCK_SIZE, SEEK_SET);
    char inode_buff[sizeof(inode_arr)];
    memset(inode_buff, 0, sizeof(inode_arr));
    fread(inode_buff, 1, sizeof(inode_arr), fp);
    memcpy(inode_arr, inode_buff, sizeof(inode_arr));

    // Storing all filenames into array
    for (int i = NO_OF_INODES - 1; i >= 0; --i)
        if (sup_block.inode_Bitmap[i] == 1)
            directory_map[dir_struct_entry[i].inode_num] = i;
        else
            free_Inode_list[i] = i;

    // Populate free_Db_list
    for (int i = DISK_BLOCKS - 1; i >= sup_block.starting_DB_Index; --i)
        if (sup_block.datablock_Bitmap[i] == 0)
            free_Db_list[i] = i;

    printf("Disk is mounted now\n");

    fclose(fp); // Close the disk image file

    return 1;
}

// Function to initialize the disk
void disk_Set() {
    char buffer[BLOCK_SIZE];    // Buffer for writing to disk

    FILE *fp = fopen(disk_name, "wb"); // Open disk image file for writing in binary mode

    memset(buffer, 0, BLOCK_SIZE);  // Initialize buffer to 0

    // Write 0 to all disk blocks
    for (int i = 0; i < DISK_BLOCKS; ++i)
        fwrite(buffer, 1, BLOCK_SIZE, fp);   // Write buffer to disk

    // Initialize super block
    memset(&sup_block, 0, sizeof(struct super_block));

    // Set bitmap for metadata blocks and data blocks
    for (int i = 0; i < sup_block.starting_DB_Index; ++i)
        sup_block.datablock_Bitmap[i] = 1;
    for (int i = sup_block.starting_DB_Index; i < DISK_BLOCKS; ++i)
        sup_block.datablock_Bitmap[i] = 0;
    for (int i = 0; i < NO_OF_INODES; ++i)
        sup_block.inode_Bitmap[i] = 0;

    // Initialize inode pointers
    for (int i = 0; i < NO_OF_INODES; ++i) {
        for (int j = 0; j < 30; j++) {
            inode_arr[i].pointer[j] = -1;
        }
    }

    // Store super block into the beginning of virtual disk
    fseek(fp, 0, SEEK_SET);
    fwrite(&sup_block, 1, sizeof(struct super_block), fp);

    // Store inode blocks after super block into virtual disk
    fseek(fp, (sup_block.startingInodesDb) * BLOCK_SIZE, SEEK_SET);
    fwrite(inode_arr, 1, sizeof(inode_arr), fp);

    fclose(fp); // Close the disk image file

    printf("Virtual Disk %s created successfully\n", disk_name);
}

// Function to boot the file system
int FS_Boot(char *path) {
    strcpy(disk_name, path);    // Copy disk image file path to global variable

    printf("FS_Boot %s\n", disk_name);

    // Check if disk image file already exists
    if (access(disk_name, F_OK) != -1) {
        printf("The Disk already exists\n");
    } else {
        // Initialize disk
        if (Disk_Init() == -1) {
            printf("Disk_Init() failed\n");
            osErrno = E_GENERAL;
            return -1;
        }

        // Save disk image
        Disk_Save(disk_name);

        // Set up disk
        disk_Set();
    }

    return 0;
}

int File_Create(char *file) {
    printf("FS_Create\n");
    return 0;
}

int File_Open(char *file) {
    printf("FS_Open\n");
    return 0;
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

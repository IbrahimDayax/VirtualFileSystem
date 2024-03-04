#include <stdio.h>      // Include standard input-output library
#include <stdlib.h>     // Include standard library
#include <string.h>     // Include string manipulation functions
#include "LibFS.h"      // Include the custom filesystem library

#define block_size 512  // Define a constant for block size

// Function to print text centered in a specified width
void print_centered(const char *text) {
    int width = 60;                 // Set the width of the output (adjust as needed)
    int len = strlen(text);         // Get the length of the input text
    int padding = (width - len) / 2; // Calculate padding needed for centering
    // Print the centered text with appropriate padding
    printf("%*s%s%*s\n", padding, "", text, padding, "");
}

// Function to print usage message and exit
void usage(char *prog) {
    fprintf(stderr, "usage: %s <disk image file>\n", prog);  // Print usage message to stderr
    exit(1);    // Exit the program with error status
}

// Main function
int main(int argc, char *argv[]) {
    if (argc != 2) {    // Check if correct number of arguments are provided
        usage(argv[0]); // Call usage function if arguments are incorrect
    }
    char *path = argv[1];  // Get the path from command line arguments
    char *buffer;          // Declare buffer variable
    int status, count, size, choice, fd, offset; // Declare variables for status, count, size, file descriptor, and offset
    char filename[16];     // Declare array to store filename
    FS_Boot(path);         // Initialize filesystem with specified path
    FS_Sync();             // Synchronize filesystem
    printf("Disk Initialized\n"); // Print message indicating disk initialization
    while (1) {                     // Start infinite loop for user interaction
        printf("*************************************************************\n");
        print_centered("Please enter your choice");  // Print centered message for user prompt
        print_centered("0. Exit");                  // Print centered menu option for exiting
        print_centered("1. File Create\t2. File Open\t3. File Read\t4. File Write");  // Print centered menu options for file operations
        print_centered("5. File Seek\t6. File Close\t7. File Unlink");                 // Print centered menu options for file operations
        print_centered("8. Dir Create\t9. Dir Read\t10. Dir Unlink");                  // Print centered menu options for directory operations
        printf("*************************************************************\n\n");
        scanf("%d", &choice);   // Read user choice

        // Menu selection cases
        switch (choice) {
            case 0: // Exit the program
                printf("closing the system\n"); // Print message indicating program termination
                exit(0);                        // Exit the program
            case 1: // File Create
                printf("Enter the file name to create\n"); // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                File_Create(filename);                      // Call function to create file
                break;
            case 2: // File Open
                printf("Enter the file name to open\n");   // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                File_Open(filename);                      // Call function to open file
                break;
            case 3: // File Read
                printf("Enter the file name to read\n");  // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                File_Read(fd, buffer, size);                      // Call function to read file
                break;
            case 4: // File Write
                printf("Enter the file name to write\n"); // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                // store_file_into_Disk(filename);
                // count=File_Write(filename, buffer,size);
                break;
            case 5: // File Seek
                File_Seek(fd, offset);                     // Call function to seek in file
                break;
            case 6: // File Close
                printf("Enter the file name to close\n"); // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                // File_Close(filename);
                break;
            case 7: // File Unlink
                printf("Enter the file name to unlink\n"); // Prompt user for filename
                scanf("%s", filename);                      // Read filename from user
                status = File_Unlink(filename);              // Call function to unlink file
                break;
            // Directory Operations
            case 8: // Dir Create
                printf("Enter the dir path to create\n"); // Prompt user for directory path
                scanf("%s", path);                          // Read directory path from user
                status = Dir_Create(path);                   // Call function to create directory
                break;
            case 9: // Dir Read
                printf("Enter the dir path to read\n");  // Prompt user for directory path
                scanf("%s", path);                          // Read directory path from user
                count = Dir_Read(path, buffer, size);          // Call function to read directory
                break;
            case 10: // Dir Unlink
                printf("Enter the dir path to unlink\n"); // Prompt user for directory path
                scanf("%s", path);                          // Read directory path from user
                status = Dir_Unlink(path);                   // Call function to unlink directory
                break;
            default: // Invalid choice
                printf("Invalid Choice!!! Try Again\n"); // Print error message for invalid choice
        }
    }

    return 0; // Return 0 to indicate successful execution
}

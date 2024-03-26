#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LibFS.h"

void err(char *prog) {
    printf("Error: %s <disk fn needed>\n", prog);
    exit(1);
}

void print_menu() {
    printf("\nMenu:\n");
    printf("1. Create file\n");
    printf("2. Create directory\n");
    printf("3. Open file\n");
    printf("4. Delete file\n");
    printf("5. Delete directory\n");
    printf("6. Exit\n");
    printf("Enter your choice: ");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        err(argv[0]);
    }

    if (FS_Boot(argv[1]) < 0) {
        printf("ERROR: can't boot file system from file '%s'\n", argv[1]);
        return -1;
    } else {
        printf("File system booted from file '%s'\n", argv[1]);
    }

    int choice;
    char path[256];
    char buf[1024];
    int fd;
    int bytes_read; // Move the declaration of bytes_read outside the switch statement

    do {
        print_menu();
        scanf("%d", &choice);
        getchar(); // Consume newline character

        switch (choice) {
            case 1:
                printf("Enter file path: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character
                if (File_Create(path) < 0) {
                    printf("Can't create file\n");
                } else {
                    printf("File created successfully\n");
                }
                break;
            case 2:
                printf("Enter directory path: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character
                if (Dir_Create(path) < 0) {
                    printf("Can't create directory\n");
                } else {
                    printf("Directory created successfully\n");
                }
                break;
            case 3:
                printf("Enter file path: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character
                fd = File_Open(path);
                if (fd < 0) {
                    printf("ERROR: can't open file '%s'\n", path);
                } else {
                    printf("File '%s' opened successfully, fd=%d\n", path, fd);
                    printf("What do you want to do with the file?\n");
                    printf("1. Write to file\n");
                    printf("2. Read from file\n");
                    printf("Enter your choice: ");
                    int action;
                    scanf("%d", &action);
                    getchar(); // Consume newline character
                    switch (action) {
                        case 1:
                            printf("Enter data to write: ");
                            fgets(buf, sizeof(buf), stdin);
                            if (File_Write(fd, buf, (int)strlen(buf)) != (int)strlen(buf)) { // Cast the result of strlen() to int
                                printf("ERROR: can't write data to fd=%d\n", fd);
                            }
                            break;
                        case 2:
                            printf("Reading from file...\n");
                            memset(buf, 0, sizeof(buf));
                            bytes_read = File_Read(fd, buf, sizeof(buf) - 1);
                            if (bytes_read < 0) {
                                printf("ERROR: can't read data from fd=%d\n", fd);
                            } else {
                                printf("Data read from file: %s\n", buf);
                            }
                            break;
                        default:
                            printf("Invalid choice\n");
                    }
                    printf("Closing file...\n");
                    if (File_Close(fd) < 0) {
                        printf("ERROR: can't close file '%s'\n", path);
                    } else {
                        printf("File '%s' closed successfully\n", path);
                    }
                }
                break;
            case 4:
                printf("Enter file path: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character
                if (File_Unlink(path) < 0) {
                    printf("ERROR: can't remove file '%s'\n", path);
                } else {
                    printf("File '%s' removed successfully\n", path);
                }
                break;
            case 5:
                printf("Enter directory path: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0'; // Remove trailing newline character
                if (Dir_Unlink(path) < 0) {
                    printf("ERROR: can't remove directory '%s'\n", path);
                } else {
                    printf("Directory '%s' removed successfully\n", path);
                }
                break;
            case 6:
                break;
            default:
                printf("Invalid choice\n");
        }
    } while (choice != 6);

    if (FS_Sync() < 0) {
        printf("ERROR: can't sync file system to file '%s'\n", argv[1]);
        return -1;
    } else {
        printf("File system sync'd to file '%s'\n", argv[1]);
    }

    return 0;
}


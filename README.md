# Virtual File System Project

Implementation of a user-level file system in C, providing a simulated file system environment within an operating system context. This project implements a basic file system in C with functionalities to create, delete, open, and write/read files and directories. The file system is simulated using a file that represents a virtual disk.

## Features

- Create and delete files and directories
- Open files and perform read/write operations
- Simple command-line interface for interacting with the file system

## Requirements

- `gcc` (GNU Compiler Collection)
- Unix-like operating system (Linux, macOS, etc.)
- Standard C library

## File System Architecture

### `LibDisk.c` & `LibDisk.h`
These files provide the abstraction layer for disk operations, facilitating interaction with simulated disk storage. `LibDisk` defines functions for:
- Initializing the disk
- Loading and saving disk contents from/to a file
- Reading and writing data to disk sectors

### `LibFS.c` & `LibFS.h`
These files implement the user-level file system library, offering functions for file and directory manipulation. `LibFS` provides operations such as:
- File creation
- File reading and writing
- File deletion
- Directory creation and deletion

### `main.c`
This file serves as the entry point for the file system program. It handles user input, interacts with the file system through `LibFS` functions, and displays output accordingly.

### `Makefile`
This file automates the build process, specifying compilation rules and dependencies to generate the executable binary. It ensures consistency in building the project and simplifies the development workflow.


## Compilation

To compile the project, use the provided `Makefile`. Open a terminal, navigate to the project directory, and run:

- ```bash
  make
This will generate an executable named main.

## Running the File System

- Run the File System
  Use the compiled executable and specify the disk image file name as follows:
  - ```bash
    ./main test
  This will start up the file system and display it's UI while using "test" as the virtual disk.
- Clean Up
  To remove the compiled files and object files, run:
  - ```bash
    make clean

## License
This project is licensed under the MIT License. See the LICENSE file for more details.




  

  

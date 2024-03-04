# Compiler to use
CC = gcc

# Compiler flags
CFLAGS = -Wall -pedantic-errors

# Rule to build the 'all' target, which depends on the 'main' target
all: main

# Rule to build the 'main' target, which depends on 'main.c', 'LibFS.o', and 'LibDisk.o'
main: main.c LibFS.o LibDisk.o
	$(CC) $(CFLAGS) -o main main.c LibFS.o LibDisk.o
    # $(CC): Invokes the C compiler (gcc in this case)
    # $(CFLAGS): Specifies the compiler flags, including warnings and error checks
    # -o main: Specifies the output file name as 'main'
    # main.c LibFS.o LibDisk.o: Dependencies of the main target

# Rule to build 'LibFS.o', which depends on 'LibFS.c' and 'LibFS.h'
LibFS.o: LibFS.c LibFS.h
	$(CC) $(CFLAGS) -c LibFS.c
    # -c: Indicates that the input files should be compiled, but not linked
    # LibFS.c: Source file for the object file
    # LibFS.h: Header file included in the source file

# Rule to build 'LibDisk.o', which depends on 'LibDisk.c' and 'LibDisk.h'
LibDisk.o: LibDisk.c LibDisk.h
	$(CC) $(CFLAGS) -c LibDisk.c
    # -c: Indicates that the input files should be compiled, but not linked
    # LibDisk.c: Source file for the object file
    # LibDisk.h: Header file included in the source file

# Rule to clean up the project directory
clean:
	rm -f main *.o
    # rm -f main *.o: Removes the main executable and all object files (*.o)

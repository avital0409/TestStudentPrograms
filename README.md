# TestStudentPrograms - File Comparison and Testing Tools
This repository contains two C programs designed for Unix-based systems, featuring file comparison and automated testing tools. These programs utilize low-level POSIX syscalls for efficient file and process management.


## Files Overview

1. **`EX1_1.c`**: A program for comparing the content of two files byte-by-byte.
2. **`EX1_2.c`**: A program for managing and testing executable C files in a directory structure, generating results for each tested file.


---

## `EX1_1.c`

### Description

This program compares two files byte-by-byte to determine if their contents are identical. It provides:
- Validation for correct input and file accessibility.
- A byte-by-byte comparison approach.
- Proper file descriptor management to ensure no resource leaks.

### How to Use

1. Compile the program using:
   ```bash
   gcc -o compare_files EX1_1.c

2. Run the program with two file paths as arguments:
   ```bash
   ./compare_files <file1_path> <file2_path>

3. Exit codes:
  - 0: Files are identical.
  - 1: Files differ.
  - 2: Files have an unknown error.


---

## `EX1_2.c`

### Description

This program automates the testing of C source files within a directory. It:
- Compiles C files within subdirectories.
- Runs the resulting executables against specified input and expected output.
- Generates a `results.csv` file with scores for each executable.

### How to Use

1. Compile the program using:
   ```bash
    gcc -o test_manager EX1_2.c

2. Prepare a configuration file with the following format (one value per line):
  - Root directory path containing subdirectories with C files.
  - Input file path for executable testing.
  - Expected output file path.

3. Run the program with the configuration file as an argument:
   ```bash
    ./test_manager <config_file_path>
  
### Outputs
- `results.csv`: Contains the results in name,score format, where name is the subdirectory name, and score is the result (100 for success, 0 for failure).


---

## Prerequisites
- **Unix-based system**: The programs are designed to work exclusively on Unix
- **GCC**: Ensure GCC is installed to compile the source files.
- **POSIX environment**: The programs use POSIX APIs like `fork`, `execvp`, and `lseek`.

---

## Notes
- Handle permissions properly for reading files and accessing directories.
- Ensure the configuration file follows the specified format for the testing tool.
- These programs heavily rely on system calls and may not work in non-Unix environments.

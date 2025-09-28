# File Copy using Pipes
This program copies the contents of one file to another using a **pipe** and **fork()**.  
The parent process reads data from the source file and writes it into the pipe, while the child process reads from the pipe and writes into the destination file.

---

## Requirements

- A Unix-like environment (Linux, macOS, WSL, etc.)
- A C compiler such as `gcc`

---

## Compilation
gcc -o filecopy filecopy.c

---

## Usage

- ./filecopy <source_file> <destination_file>
- <source_file> → The file you want to copy.
- <destination_file> → The new file where the contents will be written.

---

## Error Handling

- If the source file does not exist, the program prints an error.
- If the destination file cannot be created, an error is printed.
- If the copy fails during reading/writing, an error message is shown.

---

## Notes

- The program demonstrates inter-process communication using pipe() and fork().
- The parent process closes the read end of the pipe; the child process closes the write end.
- Buffer size is set to 25 bytes for simplicity (see BUFFER_SIZE in the code).
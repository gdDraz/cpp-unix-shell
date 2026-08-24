# C++ Unix Shell

A lightweight Unix shell implemented in C++ using POSIX system calls.

## Features

The shell currently supports:

- `cd <directory>` - change the current working directory
- `ls` - list directory contents
- `rm` - remove files
- `rm -r` - recursively remove directories
- `rm -f` - ignore missing files
- `rm -v` - verbose output
- `history [n]` - display the most recent commands
- `issue <n>` - execute a previous command
- `<program>` - execute external programs
- `<program> &` - execute programs in the background
- `<program> < input.txt` - redirect standard input
- `<program> > output.txt` - redirect standard output
- `<program> < input.txt > output.txt` - combine input and output redirection
- `quit` - exit the shell

## Technologies

- C++
- POSIX / Unix system calls
- `fork()`
- `execvp()`
- `waitpid()`
- `open()`
- `dup2()`
- `opendir()` / `readdir()`
- `stat()`

## To Do

- `>>` and `<<` support
- Implement `rmexcept <file1> <file2> ...`
- Implement command execution timeout:
  `<program> <m>` terminates the process if it runs for more than `m` seconds
- Improve command parsing and argument validation
- Handle terminated background processes and prevent zombie processes
- Add support for quoted arguments

## Compilation

```bash
g++ -Wall -Wextra -std=c++17 src/shell.cpp -o shell
# FAT16-Reader
A FAT16 disk image reader capable of reading directories and files. It makes use of the standard C library.

## Compilation
To compile the program, simply run `gcc -o fat16 src/fat16.c` in the root directory of the project.

## Usage
When running the program from the terminal, be sure to give it the location of the disk image file, for example:
`./fat16 /path/to/file.img`

The program has an interactive terminal, in which you can enter multiple commands, try `help` for more info on the available commands.

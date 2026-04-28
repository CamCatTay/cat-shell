# Cat Shell

## Operating Systems Course

### Compile and Run

```
gcc -o mysh mysh.c
./mysh
```

### Features
- Custom shell prompt (`mysh> `)
- Execute external commands (e.g., `ls`, `cat`, etc.)
- Built-in `cd` command to change directories
- Built-in `exit` command to quit the shell
- Output redirection using `>` (e.g., `ls > out.txt`)
- Input redirection using `<` (e.g., `cat < file.txt`)
- Simple piping with `|` (e.g., `ls | grep txt`)
- Defaults to home directory on `cd` with no argument
- Handles invalid commands and file errors gracefully

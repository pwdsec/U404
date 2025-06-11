# U404 Shell

U404 Shell is a lightweight command-line shell written in Rust. It provides basic file system commands and simple scripting capabilities.

## Features

- File operations: select, delete, create
- Directory creation
- System utilities: list files, display the current directory, clear the screen
- Script execution from a file
- Conditional execution with `if`, `else` and `endif`
- Math helpers: random number generation via `random` and square root calculation via `math.sqrt`

## Command Reference

| Command | Description |
|---------|-------------|
| `select-file <file>` | Select a file for operations like delete |
| `delete` | Delete the selected file |
| `makefile <file>` | Create a new file |
| `makefolder <folder>` | Create a new folder |
| `ls` | List files and directories in the current directory |
| `pwd` | Print the current working directory |
| `clear` | Clear the screen |
| `uprint <message>` | Print a Unicode string |
| `random` | Print a random number |
| `math.sqrt <number>` | Calculate the square root of a number |
| `execute_script <file>` | Execute commands from a script file |
| `help` | Show available commands |
| `exit` | Exit the shell |

## Scripting

Scripts are plain text files containing one command per line. Conditional execution is supported using `if`, `else` and `endif` statements.

Example `script.txt`:

```text
if file_exists test.txt
    uprint File exists
else
    uprint File does not exist
endif
```

Run the script with:

```sh
execute_script script.txt
```

## Building

```sh
cargo build --release
```

## Running

```sh
cargo run --release -- [script]
```

Running without arguments starts an interactive REPL.

## License

U404-Shell is open-source software released under the MIT license.

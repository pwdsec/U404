# U404 Shell

U404 Shell is a lightweight command-line shell written in Rust. It exposes handy
file management commands as well as a tiny scripting language.

## Table of Contents

- [Features](#features)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Command Reference](#command-reference)
- [Scripting](#scripting)
- [Building](#building)
- [Running](#running)
- [License](#license)

## Features

- File operations: select, delete, and create files
- Directory creation
- System utilities: list files, show the current directory and clear the screen
- Script execution from a file
- Conditional execution with `if`, `else` and `endif`
- Math helpers: random number generation via `random` and square root
  calculation via `math.sqrt`

## Quick Start

```sh
cargo run --release
```

Running without arguments starts an interactive REPL. Pass a script file to
execute it directly.

### Sample Session

```text
$ cargo run --release
> pwd
/home/user/U404
> makefile notes.txt
> ls
notes.txt
```

## Installation

Install the shell locally with Cargo:

```sh
cargo install --path u404shell
```

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

### Condition Reference

| Condition | Arguments | Description |
|-----------|-----------|-------------|
| `file_exists <file>` | Path to a file | True if the file exists |
| `is_greater <a> <b>` | Two numbers | True if `a` is greater than `b` |
| `is_even <number>` | Integer | True if the number is even |

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
cargo run --release -- script.txt
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

### U404-Shell
U404-Shell is a simple shell program written in C. <br>
It provides a command-line interface to interact with the file system and perform various operations.

### Features
* File operations: select, delete, create <br>
* Directory operations: create <br>
* System operations: list files and directories, print the current working directory, clear the screen <br>
* Script execution: execute a sequence of commands from a script file <br>
* Conditional execution: execute commands based on conditions <br>
* Math functions: random number generation and square root calculation via `math.sqrt` <br>
### Commands
* `select-file <filename>`: Select a file for operations like delete. <br>
* `delete`: Delete the selected file. <br>
* `makefile <filename>`: Create a new file. <br>
* `makefolder <foldername>`: Create a new folder. <br>
* `ls`: List files and directories in the current directory. <br>
* `pwd`: Print the current working directory. <br>
* `clear`: Clear the screen. <br>
* `uprint <message>`: Print a Unicode string. <br>
* `help`: Show available commands. <br>
### Scripting
In the U404-Shell, you can execute a script with conditional statements using the execute_script function. This function reads a script file line by line and executes each command. It also supports conditional execution of commands using `if`, `else`, and `endif` statements.

#### Here's a step-by-step guide:

1. Create a script file with your commands. Each command should be on a new line. For conditional execution, you can use `if`, `else`, and `endif` statements. The if statement should be followed by a condition. Currently, the shell supports three conditions: `file_exists`, `is_greater`, and `is_even`. <br>

Example script (script.txt):
```
if file_exists test.txt
    uprint File exists
else
    uprint File does not exist
endif
```
<br>
2. In the shell, call the execute_script function with the name of your script file as the argument.<br>
Example:

`execute_script script.txt`

This will execute the script, printing "File exists" if `test.txt` exists, and "File does not exist" otherwise.

### Building
To build the program, use a C compiler like gcc <br>
```gcc main.c -o u404shell -lm```
### Running
To run the program: `./u404shell`
To execute a script: `./u404shell script.txt`

### Contributing
Contributions are welcome. Please submit a pull request or create an issue to discuss the changes.

### License
U404-Shell is open-source software released under the MIT license.

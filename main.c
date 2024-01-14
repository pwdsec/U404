#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define getcwd _getcwd
    #define mkdir(dir, mode) _mkdir(dir)
    #define MAX_PATH_LENGTH MAX_PATH
#else
    #include <unistd.h>
    #include <dirent.h>
    #include <sys/stat.h>
    #define MAX_PATH_LENGTH 1024
#endif

#define MAX_COMMAND_LENGTH 1024
#define MAX_FILENAME_LENGTH 1024
#define MAX_CONDITION_LENGTH 1024
#define MAX_IF_DEPTH 10

char selected_file[MAX_FILENAME_LENGTH] = "";

// Function prototypes
void execute_command(const char* command);
void execute_script(char* filename);
void print_help();

// Function to execute 'ls' command
void execute_ls() {
    #ifdef _WIN32
        // Windows-specific code
        WIN32_FIND_DATA findData;
        HANDLE hFind;
        SYSTEMTIME stUTC, stLocal;
        FILETIME ftCreate, ftAccess, ftWrite;

        hFind = FindFirstFile("*", &findData);
        if (hFind == INVALID_HANDLE_VALUE) {
            printf("FindFirstFile failed (%d)\n", GetLastError());
            return;
        }

        do {
            // Convert file size to human-readable format
            LARGE_INTEGER size;
            size.HighPart = findData.nFileSizeHigh;
            size.LowPart = findData.nFileSizeLow;

            // Convert the last-write time to local time.
            FileTimeToLocalFileTime(&findData.ftLastWriteTime, &ftCreate);
            FileTimeToSystemTime(&ftCreate, &stUTC);
            SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

            printf("%-20s %-10lld %-20s\n", findData.cFileName, size.QuadPart, asctime(gmtime(&findData.ftLastWriteTime)));
        } while (FindNextFile(hFind, &findData) != 0);

        FindClose(hFind);
    #else
        // Unix-specific code
        DIR *dir;
        struct dirent *entry;
        struct stat fileStat;
        char timeBuffer[80];

        dir = opendir(".");
        if (dir == NULL) {
            perror("opendir");
            return;
        }

        printf("%-20s %-10s %-20s\n", "Name", "Size", "Last Modified");
        printf("----------------------------------------------\n");

        while ((entry = readdir(dir)) != NULL) {
            if (stat(entry->d_name, &fileStat) < 0) {
                perror("stat");
                continue;
            }

            // Convert file size to human-readable format
            double size = fileStat.st_size;
            char unit[3];
            if (size < 1024) {
                strcpy(unit, "B");
            } else if (size < pow(1024, 2)) {
                size /= 1024;
                strcpy(unit, "KB");
            } else if (size < pow(1024, 3)) {
                size /= pow(1024, 2);
                strcpy(unit, "MB");
            } else if (size < pow(1024, 4)) {
                size /= pow(1024, 3);
                strcpy(unit, "GB");
            } else {
                size /= pow(1024, 4);
                strcpy(unit, "TB");
            }

            strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", localtime(&fileStat.st_mtime));

            if (strcmp(entry->d_name, selected_file) == 0) {
                printf("\033[0;33m%-20s %.2f %s %-20s\033[0m\n", entry->d_name, size, unit, timeBuffer);
            } else {
                printf("%-20s %.2f %s %-20s\n", entry->d_name, size, unit, timeBuffer);
            }
        }

        closedir(dir);
    #endif
}

// Function to execute 'pwd' command
void execute_pwd() {
    char cwd[MAX_PATH_LENGTH];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
}

// Function to execute 'clear' command
void execute_clear() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

// Function to execute 'select-file' command
void execute_select_file(const char *filename) {
    strcpy(selected_file, filename);
}

// Function to execute 'delete' command
void execute_delete() {
    if (selected_file[0] == '\0') {
        printf("No file selected.\n");
        return;
    }

    #ifdef _WIN32
        if (DeleteFile(selected_file) != 0) {
            printf("Deleted successfully.\n");
            selected_file[0] = '\0';
        } else {
            printf("Error deleting file: %d\n", GetLastError());
        }
    #else
        if (remove(selected_file) == 0) {
            printf("Deleted successfully.\n");
            selected_file[0] = '\0';
        } else {
            perror("delete");
        }
    #endif
}

// Function to execute 'makefile' command
void execute_makefolder(const char *foldername) {
    #ifdef _WIN32
        if (CreateDirectory(foldername, NULL) != 0) {
            printf("Folder created successfully.\n");
        } else {
            printf("Error creating folder: %d\n", GetLastError());
        }
    #else
        if (mkdir(foldername, 0777) == 0) {
            printf("Folder created successfully.\n");
        } else {
            perror("mkdir");
        }
    #endif
}

// Function to execute 'makefile' command
void execute_makefile(char* filename){
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        printf("Error opening file!\n");
        return;
    }

    /* Write to file */
    fprintf(file, "This is a new file created by the U404-Shell.\n");

    /* Close file */
    fclose(file);
}

// Function to execute 'uprint' command
void execute_uprint(const char* message) {
    #ifdef _WIN32
        wprintf(L"%s\n", message);
    #else
        printf("%s\n", message);
    #endif
}

// Function to check if a file exists
bool file_exists(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file != NULL) {
        fclose(file);
        return true;
    }
    return false;
}

// Function to check if a is greater than b
bool is_greater(int a, int b) {
    return a > b;
}

// Function to check if a number is even
bool is_even(int a) {
    return a % 2 == 0;
}

// Function to check a condition
bool check_condition(const char* condition) {
    char condCopy[1024];
    strncpy(condCopy, condition, 1023);
    condCopy[1023] = '\0';  // Ensure null-termination

    char* token = strtok(condCopy, " ");
    if (token != NULL) {
        if (strcmp(token, "file_exists") == 0) {
            token = strtok(NULL, " ");
            return file_exists(token);
        } else if (strcmp(token, "is_greater") == 0) {
            token = strtok(NULL, " ");
            int a = atoi(token);
            token = strtok(NULL, " ");
            int b = atoi(token);
            return is_greater(a, b);
        } else if (strcmp(token, "is_even") == 0) {
            token = strtok(NULL, " ");
            int a = atoi(token);
            return is_even(a);
        }
        // Add more conditions here
    }
    return false; // Default to false if condition is unrecognized
}

// Function to trim newline character from a string
void trim_newline(char* str) {
    if (str == NULL) return;
    str[strcspn(str, "\n")] = 0;
}

// Function to execute a command
void execute_command(const char* command) {
    if (strcmp(command, "ls") == 0) {
        execute_ls();
    } else if (strncmp(command, "select-file ", 12) == 0) {
        execute_select_file(command + 12);
    } else if (strcmp(command, "delete") == 0) {
        execute_delete();
    } else if (strcmp(command, "pwd") == 0) {
        execute_pwd();
    } else if (strcmp(command, "clear") == 0) {
        execute_clear();
    } else if (strncmp(command, "makefolder ", 11) == 0) {
        execute_makefolder(command + 11);
    } else if (strncmp(command, "makefile ", 9) == 0) {
        execute_makefile(command + 9);
    } else if (strncmp(command, "uprint ", 7) == 0) {
        execute_uprint(command + 7);
    } else if (strcmp(command, "help") == 0) {
        print_help();
    } else if (strncmp(command, "execute_script ", 15) == 0) {
        execute_script(command + 15);
    } else {
        printf("Unknown command: %s\n", command);
    }
}

// Function to execute a script
void execute_script(char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open script file");
        return;
    }

    char command[1024];
    int if_depth = 0; // Track nesting level of if statements
    bool condition_stack[10] = {false}; // Stack to track conditions for nested if statements
    bool execute_command_flag = true; // Flag to determine whether to execute a command

    while (fgets(command, sizeof(command), file)) {
        trim_newline(command);

        if (strncmp(command, "if ", 3) == 0) {
            if (if_depth < 10) {
                execute_command_flag = (if_depth == 0) || (condition_stack[if_depth - 1] && execute_command_flag);
                condition_stack[if_depth++] = check_condition(command + 3) && execute_command_flag;
            } else {
                fprintf(stderr, "Error: Nested if statements too deep\n");
                break;
            }
        } else if (strcmp(command, "else") == 0) {
            if (if_depth > 0) {
                condition_stack[if_depth - 1] = !condition_stack[if_depth - 1] && execute_command_flag;
            } else {
                fprintf(stderr, "Error: else without matching if\n");
            }
        } else if (strcmp(command, "endif") == 0) {
            if (if_depth > 0) {
                if_depth--;
                execute_command_flag = (if_depth == 0) || condition_stack[if_depth - 1];
            } else {
                fprintf(stderr, "Error: endif without matching if\n");
            }
        } else if (if_depth == 0 || condition_stack[if_depth - 1]) {
            execute_command(command);
        }
    }

    fclose(file);
}

void print_help() {
    printf("Available commands:\n");
    printf("File operations:\n");
    printf("  select-file <filename> - Select a file\n");
    printf("  delete - Delete the selected file\n");
    printf("  makefile <filename> - Create a new file\n");
    printf("Folder operations:\n");
    printf("  makefolder - Create a new folder\n");
    printf("System operations:\n");
    printf("  ls - List files and directories\n");
    printf("  pwd - Print the current working directory\n");
    printf("  clear - Clear the screen\n");
    printf("General:\n");
    printf("  help - Show available commands\n");
}

// Main function
int main() {
    char command[MAX_COMMAND_LENGTH];

    while (1) {
        printf("U404-Shell> ");
        fgets(command, MAX_COMMAND_LENGTH, stdin);

        // Remove newline character
        command[strcspn(command, "\n")] = 0;

        execute_command(command);
    }

    return 0;
}

#ifndef SHELL_H
#define SHELL_H

#include <stdbool.h>

#define MAX_TOKEN_LENGTH 256
#define MAX_TOKENS 1024
#define MAX_VARIABLES 1000
#define MAX_FUNCTIONS 100
#define MAX_STACK_SIZE 1000

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_OPERATOR,
    TOKEN_PUNCTUATION,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_TOKEN_LENGTH];
} Token;

typedef enum {
    VAR_NIL,
    VAR_NUMBER,
    VAR_STRING,
    VAR_BOOLEAN,
    VAR_TABLE,
    VAR_FUNCTION
} VariableType;

typedef struct TableEntry TableEntry;

struct TableEntry {
    char* key;
    struct Variable* value;
    TableEntry* next;
};

typedef struct Variable {
    VariableType type;
    union {
        double number;
        char* string;
        bool boolean;
        TableEntry* table;
        int function_index;
    } value;
} Variable;

typedef struct {
    char name[MAX_TOKEN_LENGTH];
    Variable value;
} VariableEntry;

typedef struct {
    char name[MAX_TOKEN_LENGTH];
    int start_index;
    int end_index;
} Function;

/* Global variables declared in main.c */
extern Token tokens[MAX_TOKENS];
extern int token_count;
extern int current_token;

extern VariableEntry variables[MAX_VARIABLES];
extern int variable_count;

extern Function functions[MAX_FUNCTIONS];
extern int function_count;

extern Variable stack[MAX_STACK_SIZE];
extern int stack_top;

void tokenize(const char* input);
void parse_and_execute();
void execute_statement();
Variable evaluate_expression();
Variable evaluate_addition();
Variable evaluate_term();
Variable evaluate_factor();
Variable execute_function(const char* name);
void error(const char* message);
void push(Variable value);
Variable pop();
Variable* get_variable(const char* name);
void set_variable(const char* name, Variable value);
void print_variable(Variable var);

Variable func_print();
Variable func_type();
Variable func_tonumber();
Variable func_tostring();
Variable func_math_random();
Variable func_math_sqrt();
Variable func_os_time();
Variable func_os_clock();
Variable func_string_len();
Variable func_string_sub();
Variable func_table_insert();
Variable func_table_remove();

void compile_script(const char* script_file, const char* bytecode_file);
void run_bytecode(const char* bytecode_file);

#endif /* SHELL_H */

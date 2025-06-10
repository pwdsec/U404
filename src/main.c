#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep(x * 1000)
#else
    #include <unistd.h>
#endif

#include "shell.h"

Token tokens[MAX_TOKENS];
int token_count = 0;
int current_token = 0;

VariableEntry variables[MAX_VARIABLES];
int variable_count = 0;

Function functions[MAX_FUNCTIONS];
int function_count = 0;

Variable stack[MAX_STACK_SIZE];
int stack_top = -1;


void tokenize(const char* input) {
    token_count = 0;
    char* token_start = (char*)input;
    char* p = (char*)input;

    while (*p) {
        if (isspace(*p)) {
            p++;
            token_start = p;
        } else if (isalpha(*p) || *p == '_') {
            while (isalnum(*p) || *p == '_') p++;
            int length = p - token_start;
            if (length > MAX_TOKEN_LENGTH - 1) length = MAX_TOKEN_LENGTH - 1;
            strncpy(tokens[token_count].value, token_start, length);
            tokens[token_count].value[length] = '\0';
            
            if (strcmp(tokens[token_count].value, "if") == 0 ||
                strcmp(tokens[token_count].value, "then") == 0 ||
                strcmp(tokens[token_count].value, "else") == 0 ||
                strcmp(tokens[token_count].value, "elseif") == 0 ||
                strcmp(tokens[token_count].value, "end") == 0 ||
                strcmp(tokens[token_count].value, "while") == 0 ||
                strcmp(tokens[token_count].value, "do") == 0 ||
                strcmp(tokens[token_count].value, "for") == 0 ||
                strcmp(tokens[token_count].value, "function") == 0 ||
                strcmp(tokens[token_count].value, "local") == 0 ||
                strcmp(tokens[token_count].value, "return") == 0 ||
                strcmp(tokens[token_count].value, "break") == 0 ||
                strcmp(tokens[token_count].value, "nil") == 0 ||
                strcmp(tokens[token_count].value, "true") == 0 ||
                strcmp(tokens[token_count].value, "false") == 0) {
                tokens[token_count].type = TOKEN_KEYWORD;
            } else {
                tokens[token_count].type = TOKEN_IDENTIFIER;
            }
            token_count++;
            token_start = p;
        } else if (isdigit(*p) || (*p == '.' && isdigit(*(p+1)))) {
            while (isdigit(*p) || *p == '.') p++;
            int length = p - token_start;
            if (length > MAX_TOKEN_LENGTH - 1) length = MAX_TOKEN_LENGTH - 1;
            strncpy(tokens[token_count].value, token_start, length);
            tokens[token_count].value[length] = '\0';
            tokens[token_count].type = TOKEN_NUMBER;
            token_count++;
            token_start = p;
        } else if (*p == '"' || *p == '\'') {
            char quote = *p;
            p++;
            token_start = p;
            while (*p && *p != quote) p++;
            if (*p == quote) {
                int length = p - token_start;
                if (length > MAX_TOKEN_LENGTH - 1) length = MAX_TOKEN_LENGTH - 1;
                strncpy(tokens[token_count].value, token_start, length);
                tokens[token_count].value[length] = '\0';
                tokens[token_count].type = TOKEN_STRING;
                token_count++;
                p++;
            } else {
                error("Unterminated string literal");
            }
            token_start = p;
        } else if (*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '%' || *p == '^' ||
                   *p == '=' || *p == '<' || *p == '>' || *p == '~') {
            if ((*p == '=' && *(p+1) == '=') || (*p == '~' && *(p+1) == '=') ||
                (*p == '<' && *(p+1) == '=') || (*p == '>' && *(p+1) == '=')) {
                tokens[token_count].value[0] = *p;
                tokens[token_count].value[1] = *(p+1);
                tokens[token_count].value[2] = '\0';
                p += 2;
            } else {
                tokens[token_count].value[0] = *p;
                tokens[token_count].value[1] = '\0';
                p++;
            }
            tokens[token_count].type = TOKEN_OPERATOR;
            token_count++;
            token_start = p;
        } else if (*p == '(' || *p == ')' || *p == '{' || *p == '}' || *p == '[' || *p == ']' ||
                   *p == ',' || *p == ';') {
            tokens[token_count].value[0] = *p;
            tokens[token_count].value[1] = '\0';
            tokens[token_count].type = TOKEN_PUNCTUATION;
            token_count++;
            p++;
            token_start = p;
        } else {
            error("Invalid character in input");
        }
    }

    tokens[token_count].type = TOKEN_EOF;
    strcpy(tokens[token_count].value, "EOF");
    token_count++;
}

void parse_and_execute() {
    current_token = 0;
    while (tokens[current_token].type != TOKEN_EOF) {
        execute_statement();
    }
}

void execute_statement() {
    if (tokens[current_token].type == TOKEN_KEYWORD) {
        if (strcmp(tokens[current_token].value, "if") == 0) {
            current_token++;
            Variable condition = evaluate_expression();
            if (tokens[current_token].type != TOKEN_KEYWORD || strcmp(tokens[current_token].value, "then") != 0) {
                error("Expected 'then' after if condition");
            }
            current_token++;
            if (condition.type == VAR_BOOLEAN && condition.value.boolean) {
                while (tokens[current_token].type != TOKEN_KEYWORD || 
                       (strcmp(tokens[current_token].value, "else") != 0 &&
                        strcmp(tokens[current_token].value, "elseif") != 0 &&
                        strcmp(tokens[current_token].value, "end") != 0)) {
                    execute_statement();
                }
            } else {
                int nesting = 1;
                while (nesting > 0) {
                    if (tokens[current_token].type == TOKEN_KEYWORD) {
                        if (strcmp(tokens[current_token].value, "if") == 0) nesting++;
                        else if (strcmp(tokens[current_token].value, "end") == 0) nesting--;
                        else if (nesting == 1 && (strcmp(tokens[current_token].value, "else") == 0 || 
                                                  strcmp(tokens[current_token].value, "elseif") == 0)) {
                            break;
                        }
                    }
                    current_token++;
                }
            }
            if (tokens[current_token].type == TOKEN_KEYWORD && strcmp(tokens[current_token].value, "else") == 0) {
                current_token++;
                if (!condition.value.boolean) {
                    while (tokens[current_token].type != TOKEN_KEYWORD || strcmp(tokens[current_token].value, "end") != 0) {
                        execute_statement();
                    }
                }
            }
            while (tokens[current_token].type != TOKEN_KEYWORD || strcmp(tokens[current_token].value, "end") != 0) {
                current_token++;
            }
            current_token++;
        } else if (strcmp(tokens[current_token].value, "while") == 0) {
            int loop_start = current_token;
            current_token++;
            Variable condition = evaluate_expression();
            if (tokens[current_token].type != TOKEN_KEYWORD || strcmp(tokens[current_token].value, "do") != 0) {
                error("Expected 'do' after while condition");
            }
            current_token++;
            while (condition.type == VAR_BOOLEAN && condition.value.boolean) {
                while (tokens[current_token].type != TOKEN_KEYWORD || strcmp(tokens[current_token].value, "end") != 0) {
                    execute_statement();
                }
                current_token = loop_start + 1;
                condition = evaluate_expression();
                current_token++;  // Skip 'do'
            }
            int nesting = 1;
            while (nesting > 0) {
                if (tokens[current_token].type == TOKEN_KEYWORD) {
                    if (strcmp(tokens[current_token].value, "while") == 0) nesting++;
                    else if (strcmp(tokens[current_token].value, "end") == 0) nesting--;
                }
                current_token++;
            }
        } else if (strcmp(tokens[current_token].value, "function") == 0) {
            current_token++;
            if (tokens[current_token].type != TOKEN_IDENTIFIER) {
                error("Expected function name");
            }
            char function_name[MAX_TOKEN_LENGTH];
            strcpy(function_name, tokens[current_token].value);
            current_token++;
            if (tokens[current_token].type != TOKEN_PUNCTUATION || strcmp(tokens[current_token].value, "(") != 0) {
                error("Expected '(' after function name");
            }
            current_token++;
            if (tokens[current_token].type != TOKEN_PUNCTUATION || strcmp(tokens[current_token].value, ")") != 0) {
                error("Expected ')' after function parameters");
            }
            current_token++;
            int function_start = current_token;
            int nesting = 1;
            while (nesting > 0) {
                if (tokens[current_token].type == TOKEN_KEYWORD) {
                    if (strcmp(tokens[current_token].value, "function") == 0) nesting++;
                    else if (strcmp(tokens[current_token].value, "end") == 0) nesting--;
                }
                current_token++;
            }
            int function_end = current_token - 1;
            
            if (function_count < MAX_FUNCTIONS) {
                strcpy(functions[function_count].name, function_name);
                functions[function_count].start_index = function_start;
                functions[function_count].end_index = function_end;
                function_count++;
            } else {
                error("Too many functions defined");
            }
        } else if (strcmp(tokens[current_token].value, "local") == 0) {
            current_token++;
            if (tokens[current_token].type != TOKEN_IDENTIFIER) {
                error("Expected variable name after 'local'");
            }
            char var_name[MAX_TOKEN_LENGTH];
            strcpy(var_name, tokens[current_token].value);
            current_token++;
            if (tokens[current_token].type != TOKEN_OPERATOR || strcmp(tokens[current_token].value, "=") != 0) {
                error("Expected '=' after variable name in local declaration");
            }
            current_token++;
            Variable value = evaluate_expression();
            set_variable(var_name, value);
        }
    } else if (tokens[current_token].type == TOKEN_IDENTIFIER) {
        char var_name[MAX_TOKEN_LENGTH];
        strcpy(var_name, tokens[current_token].value);
        current_token++;
        if (tokens[current_token].type == TOKEN_OPERATOR && strcmp(tokens[current_token].value, "=") == 0) {
            current_token++;
            Variable value = evaluate_expression();
            set_variable(var_name, value);
        } else {
            current_token--;
            Variable result = evaluate_expression();
            // Discard the result if it's not used
        }
    } else {
        error("Unexpected token at start of statement");
    }
}

Variable evaluate_expression() {
    Variable left = evaluate_term();
    while (tokens[current_token].type == TOKEN_OPERATOR &&
           (strcmp(tokens[current_token].value, "+") == 0 || strcmp(tokens[current_token].value, "-") == 0)) {
        char op = tokens[current_token].value[0];
        current_token++;
        Variable right = evaluate_term();
        if (left.type == VAR_NUMBER && right.type == VAR_NUMBER) {
            if (op == '+') {
                left.value.number += right.value.number;
            }else {
                left.value.number -= right.value.number;
            }
        } else if (left.type == VAR_STRING && right.type == VAR_STRING && op == '+') {
            char* new_string = malloc(strlen(left.value.string) + strlen(right.value.string) + 1);
            strcpy(new_string, left.value.string);
            strcat(new_string, right.value.string);
            free(left.value.string);
            left.value.string = new_string;
        } else {
            error("Invalid operands for '+' or '-'");
        }
    }
    return left;
}

Variable evaluate_term() {
    Variable left = evaluate_factor();
    while (tokens[current_token].type == TOKEN_OPERATOR &&
           (strcmp(tokens[current_token].value, "*") == 0 || strcmp(tokens[current_token].value, "/") == 0 ||
            strcmp(tokens[current_token].value, "%") == 0)) {
        char op = tokens[current_token].value[0];
        current_token++;
        Variable right = evaluate_factor();
        if (left.type == VAR_NUMBER && right.type == VAR_NUMBER) {
            if (op == '*') {
                left.value.number *= right.value.number;
            } else if (op == '/') {
                if (right.value.number == 0) {
                    error("Division by zero");
                }
                left.value.number /= right.value.number;
            } else {
                left.value.number = fmod(left.value.number, right.value.number);
            }
        } else {
            error("Invalid operands for '*', '/' or '%'");
        }
    }
    return left;
}

Variable evaluate_factor() {
    if (tokens[current_token].type == TOKEN_NUMBER) {
        Variable var;
        var.type = VAR_NUMBER;
        var.value.number = atof(tokens[current_token].value);
        current_token++;
        return var;
    } else if (tokens[current_token].type == TOKEN_STRING) {
        Variable var;
        var.type = VAR_STRING;
        var.value.string = strdup(tokens[current_token].value);
        current_token++;
        return var;
    } else if (tokens[current_token].type == TOKEN_KEYWORD) {
        if (strcmp(tokens[current_token].value, "true") == 0) {
            Variable var;
            var.type = VAR_BOOLEAN;
            var.value.boolean = true;
            current_token++;
            return var;
        } else if (strcmp(tokens[current_token].value, "false") == 0) {
            Variable var;
            var.type = VAR_BOOLEAN;
            var.value.boolean = false;
            current_token++;
            return var;
        } else if (strcmp(tokens[current_token].value, "nil") == 0) {
            Variable var;
            var.type = VAR_NIL;
            current_token++;
            return var;
        }
    } else if (tokens[current_token].type == TOKEN_IDENTIFIER) {
        char var_name[MAX_TOKEN_LENGTH];
        strcpy(var_name, tokens[current_token].value);
        current_token++;
        if (tokens[current_token].type == TOKEN_PUNCTUATION && strcmp(tokens[current_token].value, "(") == 0) {
            current_token++;
            // Function call
            if (strcmp(var_name, "print") == 0) {
                return func_print();
            } else if (strcmp(var_name, "type") == 0) {
                return func_type();
            } else if (strcmp(var_name, "tonumber") == 0) {
                return func_tonumber();
            } else if (strcmp(var_name, "tostring") == 0) {
                return func_tostring();
            } else if (strcmp(var_name, "math.random") == 0) {
                return func_math_random();
            } else if (strcmp(var_name, "math.sqrt") == 0) {
                return func_math_sqrt();
            } else if (strcmp(var_name, "os.time") == 0) {
                return func_os_time();
            } else if (strcmp(var_name, "os.clock") == 0) {
                return func_os_clock();
            } else if (strcmp(var_name, "string.len") == 0) {
                return func_string_len();
            } else if (strcmp(var_name, "string.sub") == 0) {
                return func_string_sub();
            } else if (strcmp(var_name, "table.insert") == 0) {
                return func_table_insert();
            } else if (strcmp(var_name, "table.remove") == 0) {
                return func_table_remove();
            } else {
                return execute_function(var_name);
            }
        } else {
            Variable* var = get_variable(var_name);
            if (var == NULL) {
                error("Undefined variable");
            }
            return *var;
        }
    } else if (tokens[current_token].type == TOKEN_PUNCTUATION && strcmp(tokens[current_token].value, "(") == 0) {
        current_token++;
        Variable var = evaluate_expression();
        if (tokens[current_token].type != TOKEN_PUNCTUATION || strcmp(tokens[current_token].value, ")") != 0) {
            error("Expected ')'");
        }
        current_token++;
        return var;
    }
    error("Unexpected token in factor");
    Variable dummy;
    dummy.type = VAR_NIL;
    return dummy;
}

void error(const char* message) {
    fprintf(stderr, "Error: %s\n", message);
    exit(1);
}

void push(Variable value) {
    if (stack_top < MAX_STACK_SIZE - 1) {
        stack[++stack_top] = value;
    } else {
        error("Stack overflow");
    }
}

Variable pop() {
    if (stack_top >= 0) {
        return stack[stack_top--];
    } else {
        error("Stack underflow");
        Variable dummy;
        dummy.type = VAR_NIL;
        return dummy;
    }
}

Variable* get_variable(const char* name) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            return &variables[i].value;
        }
    }
    return NULL;
}

void set_variable(const char* name, Variable value) {
    for (int i = 0; i < variable_count; i++) {
        if (strcmp(variables[i].name, name) == 0) {
            variables[i].value = value;
            return;
        }
    }
    if (variable_count < MAX_VARIABLES) {
        strcpy(variables[variable_count].name, name);
        variables[variable_count].value = value;
        variable_count++;
    } else {
        error("Too many variables");
    }
}
Variable execute_function(const char* name) {
    for (int i = 0; i < function_count; i++) {
        if (strcmp(functions[i].name, name) == 0) {
            int saved_token = current_token;
            int start = functions[i].start_index;
            int end = functions[i].end_index;
            current_token = start;
            while (current_token <= end) {
                execute_statement();
            }
            current_token = saved_token;
            Variable result;
            result.type = VAR_NIL;
            return result;
        }
    }
    error("Undefined function");
    Variable dummy;
    dummy.type = VAR_NIL;
    return dummy;
}


void print_variable(Variable var) {
    switch (var.type) {
        case VAR_NIL:
            printf("nil");
            break;
        case VAR_NUMBER:
            printf("%g", var.value.number);
            break;
        case VAR_STRING:
            printf("%s", var.value.string);
            break;
        case VAR_BOOLEAN:
            printf(var.value.boolean ? "true" : "false");
            break;
        case VAR_TABLE:
            printf("table: %p", (void*)var.value.table);
            break;
        case VAR_FUNCTION:
            printf("function: %d", var.value.function_index);
            break;
    }
}

// Built-in functions implementation

Variable func_print() {
    Variable arg = pop();
    print_variable(arg);
    printf("\n");
    Variable result;
    result.type = VAR_NIL;
    return result;
}

Variable func_type() {
    Variable arg = pop();
    Variable result;
    result.type = VAR_STRING;
    switch (arg.type) {
        case VAR_NIL: result.value.string = strdup("nil"); break;
        case VAR_NUMBER: result.value.string = strdup("number"); break;
        case VAR_STRING: result.value.string = strdup("string"); break;
        case VAR_BOOLEAN: result.value.string = strdup("boolean"); break;
        case VAR_TABLE: result.value.string = strdup("table"); break;
        case VAR_FUNCTION: result.value.string = strdup("function"); break;
    }
    return result;
}

Variable func_tonumber() {
    Variable arg = pop();
    Variable result;
    result.type = VAR_NUMBER;
    if (arg.type == VAR_STRING) {
        result.value.number = atof(arg.value.string);
    } else if (arg.type == VAR_NUMBER) {
        result = arg;
    } else {
        result.type = VAR_NIL;
    }
    return result;
}

Variable func_tostring() {
    Variable arg = pop();
    Variable result;
    result.type = VAR_STRING;
    char buffer[64];
    switch (arg.type) {
        case VAR_NIL:
            result.value.string = strdup("nil");
            break;
        case VAR_NUMBER:
            snprintf(buffer, sizeof(buffer), "%g", arg.value.number);
            result.value.string = strdup(buffer);
            break;
        case VAR_STRING:
            result = arg;
            break;
        case VAR_BOOLEAN:
            result.value.string = strdup(arg.value.boolean ? "true" : "false");
            break;
        case VAR_TABLE:
            snprintf(buffer, sizeof(buffer), "table: %p", (void*)arg.value.table);
            result.value.string = strdup(buffer);
            break;
        case VAR_FUNCTION:
            snprintf(buffer, sizeof(buffer), "function: %d", arg.value.function_index);
            result.value.string = strdup(buffer);
            break;
    }
    return result;
}

Variable func_math_random() {
    Variable result;
    result.type = VAR_NUMBER;
    result.value.number = (double)rand() / RAND_MAX;
    return result;
}
Variable func_math_sqrt() {
    Variable arg = pop();
    Variable result;
    result.type = VAR_NUMBER;
    if (arg.type == VAR_NUMBER && arg.value.number >= 0) {
        result.value.number = sqrt(arg.value.number);
    } else {
        result.type = VAR_NIL;
    }
    return result;
}


Variable func_os_time() {
    Variable result;
    result.type = VAR_NUMBER;
    result.value.number = (double)time(NULL);
    return result;
}

Variable func_os_clock() {
    Variable result;
    result.type = VAR_NUMBER;
    result.value.number = (double)clock() / CLOCKS_PER_SEC;
    return result;
}

Variable func_string_len() {
    Variable arg = pop();
    Variable result;
    result.type = VAR_NUMBER;
    if (arg.type == VAR_STRING) {
        result.value.number = strlen(arg.value.string);
    } else {
        result.type = VAR_NIL;
    }
    return result;
}

Variable func_string_sub() {
    Variable end = pop();
    Variable start = pop();
    Variable str = pop();
    Variable result;
    result.type = VAR_STRING;
    if (str.type == VAR_STRING && start.type == VAR_NUMBER && end.type == VAR_NUMBER) {
        int s = (int)start.value.number - 1;  // Lua uses 1-based indexing
        int e = (int)end.value.number;
        if (s < 0) s = 0;
        if (e > strlen(str.value.string)) e = strlen(str.value.string);
        if (s < e) {
            result.value.string = malloc(e - s + 1);
            strncpy(result.value.string, str.value.string + s, e - s);
            result.value.string[e - s] = '\0';
        } else {
            result.value.string = strdup("");
        }
    } else {
        result.type = VAR_NIL;
    }
    return result;
}

Variable func_table_insert() {
    // This is a simplified version that always inserts at the end
    Variable value = pop();
    Variable table = pop();
    if (table.type != VAR_TABLE) {
        error("First argument to table.insert must be a table");
    }
    TableEntry* new_entry = malloc(sizeof(TableEntry));
    new_entry->key = NULL;  // For array-like behavior
    new_entry->value = malloc(sizeof(Variable));
    *new_entry->value = value;
    new_entry->next = NULL;
    
    if (table.value.table == NULL) {
        table.value.table = new_entry;
    } else {
        TableEntry* last = table.value.table;
        while (last->next != NULL) {
            last = last->next;
        }
        last->next = new_entry;
    }
    
    Variable result;
    result.type = VAR_NIL;
    return result;
}

Variable func_table_remove() {
    // This is a simplified version that always removes from the end
    Variable table = pop();
    if (table.type != VAR_TABLE) {
        error("Argument to table.remove must be a table");
    }
    
    Variable result;
    result.type = VAR_NIL;
    
    if (table.value.table == NULL) {
        return result;
    }
    
    if (table.value.table->next == NULL) {
        result = *table.value.table->value;
        free(table.value.table->value);
        free(table.value.table);
        table.value.table = NULL;
    } else {
        TableEntry* second_last = table.value.table;
        while (second_last->next->next != NULL) {
            second_last = second_last->next;
        }
        result = *second_last->next->value;
        free(second_last->next->value);
        free(second_last->next);
        second_last->next = NULL;
    }
    
    return result;
}

int main() {
    char input[1000];
    
    printf("Lua-like Shell\n");
    printf("Type 'exit' to quit\n");
    
    while (1) {
        printf("> ");
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        if (strcmp(input, "exit\n") == 0) {
            break;
        }
        
        tokenize(input);
        parse_and_execute();
    }
    
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "token.h"

int line_number = 1;


static void read_identifier(FILE *j_file, char *buffer) {
    int i = 0; 
    int character;
    while ((character = fgetc(j_file)) != EOF && (isalnum(character) || character == '_')) {
        if (i < MAX_TOKEN_LENGTH) {
            buffer[i++] = character;
        }
    }
    buffer[i] = '\0';
    if (character != EOF) {
        ungetc(character, j_file);
    }
}

bool next_token(FILE *j_file, token *output) {
    if (!j_file || !output) return false;

    memset(output, 0, sizeof(token));

    while (1) {
        int character;
        do {
            character = fgetc(j_file);
            if (character == '\n') line_number++;
        } while (character != EOF && isspace(character));

        if (character == EOF) {
            return false; 
        }

        if (character == ';') {
            while ((character = fgetc(j_file)) != EOF && character != '\n') {
            }
            if (character == '\n') {
                line_number++;
            }
            continue;
        }

        ungetc(character, j_file);
        break;
    }

    int first_char = fgetc(j_file);
    if (first_char == EOF) {
        return false;
    }

    if (isalpha(first_char)) {
        ungetc(first_char, j_file);
        read_identifier(j_file, output -> str);

        if (strcmp(output ->str, "defun") == 0) {
            output -> type = DEFUN;
        } else if (strcmp(output->str, "return") == 0) { 
            output -> type = RETURN;
        } else if (strcmp(output->str, "if") == 0) {
             output -> type = IF;
        } else if (strcmp(output -> str, "else") == 0) {
            output -> type = ELSE;
        } else if (strcmp(output -> str, "endif") == 0) {
            output -> type = ENDIF;
        } else if (strcmp(output -> str, "while") == 0) {
            output -> type = WHILE;
        }else if (strcmp(output -> str, "endwhile") == 0) {
            output->type = ENDWHILE;
        } else if (strncmp(output-> str, "get_arg", 7) == 0) {
            output -> type = GET_ARG;
            output -> arg_no = atoi(output->str + 7);
        } else if (strncmp(output->str, "set_arg", 7) == 0) {
            output -> type = SET_ARG;
            output -> arg_no = atoi(output -> str + 7);

        } else if (strcmp(output->str, "and") == 0) {
            output->type = AND;
        } else if (strcmp(output->str, "or") == 0) {
            output->type = OR;
        } else if (strcmp(output->str, "not") == 0) {
            output->type = NOT;
        } else if (strcmp(output->str, "lt") == 0) {
            output->type = LT;
        } else if (strcmp(output->str, "le") == 0) {
            output->type = LE;
        } else if (strcmp(output->str, "eq") == 0) {
            output->type = EQ;
        } else if (strcmp(output->str, "ge") == 0) {
            output->type = GE;
        } else if (strcmp(output->str, "gt") == 0) {
            output->type = GT;
        } else if (strcmp(output->str, "drop") == 0) { 
            output->type = DROP;
        } else if (strcmp(output->str, "dup") == 0) {
            output->type = DUP;
        } else if (strcmp(output->str, "swap") == 0) {
            output->type = SWAP;
        } else if (strcmp(output->str, "rot") == 0) {
            output->type = ROT;
        } else {
            output->type = IDENT;
        }

        return true;
    }

    if (isdigit(first_char) || first_char == '-') {
        char buffer[MAX_TOKEN_LENGTH + 1];
        int i = 0;
        buffer[i++] = (char) first_char;


        int character;
        while ((character = fgetc(j_file)) != EOF) {
            if (isdigit(character) || character == 'x' || character == 'X' ||
                (character >= 'a' && character <= 'f') || (character >= 'A' && 
                character <= 'F')) {
                if (i < MAX_TOKEN_LENGTH) {
                    buffer[i++] = (char) character;
                } else {

                    ungetc(character, j_file);
                    output -> type = BAD_TOKEN;
                    return false;
                }
            } else {

                ungetc(character, j_file);
                break;
            }
        }

        buffer[i] = '\0';
        char *end_ptr;
        long val = strtol(buffer, &end_ptr, 0);

        if (end_ptr == buffer) {
            if (strcmp(buffer, "-") == 0) {
                output->type = MINUS;
                return true;
            }
            output -> type = BAD_TOKEN;
            return false;
        }
        output -> literal_value = (int32_t) val;
        output -> type = LITERAL;
        return true;
    }

    if (first_char == '+') {
        output -> type = PLUS;
        return true;
    } else if (first_char == '*') {
        output -> type = MUL;
        return true;
    } else if (first_char == '/') {
        output -> type = DIV;
        return true;
    } else if (first_char == '%') {
        output -> type = MOD;
        return true;
    }

    output -> type = BAD_TOKEN;
    return false;
}

void init_token(token *t) {
    if (t) {
        t->type = BAD_TOKEN;
        t->literal_value = 0;
        t->arg_no = 0;
        t->str[0] = '\0';
    }
}

//optional
void print_token (FILE* f, token to_print) {
    return;
}

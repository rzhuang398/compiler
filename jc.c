#include "token.h"
#include "asm_gen.h"
#include "stack.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>



int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE* j_file = fopen(argv[1], "r");
    if (!j_file) {
        fprintf(stderr, "cannot open input file %s\n", argv[1]);
        return 1;
    }

     char *fileName = strtok(argv[1], ".");
    char start[256] = ".";
    char extension[] = ".s";
     strncat(fileName, extension, 2);
    strncat(start, fileName, strlen(fileName));

    FILE *output = fopen(start, "w");
    
    if (!output) {
        fprintf(stderr, "cannot open output file %s\n", start);
        fclose(j_file);
        exit(1);
    }

    asm_setup(output);

    token* current_tok = (token*)malloc(sizeof(token));

    if (!current_tok) {
        fprintf(stderr, "cannot allocate memory, allocation failed\n");
        fclose(j_file);
         fclose(output);
        exit(1);
    }

    init_token(current_tok);

    bool infunc = false;
    int while_count = 0;
    int_stack* while_stack = int_stack_create();
    int if_count = 0;
    int_stack* if_stack = int_stack_create();
    int_stack* else_stack = int_stack_create();
    int i = 0;

    while (next_token(j_file, current_tok)) {

        if (current_tok->type == DEFUN || current_tok->type == RETURN) {
            if (current_tok->type == DEFUN) {

                if (!next_token(j_file, current_tok)) {
                    fprintf(stderr, "no function name\n");
                     break;
                }

                if (!infunc) {

                    infunc = true;
                    fprintf(output, "\t.globl %s\n", current_tok->str);
                    fprintf(output, "\t.p2align\t2\n");
                    fprintf(output, "\t.type\t%s,@function\n", current_tok->str);
                    fprintf(output, "%s:\n", current_tok->str);
                    asm_enter_func(output);

                } else {
                    printf("invalid definition\n");
                }
            } else if (current_tok->type == RETURN) {
                if (infunc) {
                     stack_pop_asm(output, 'a', 0);
                    infunc = false;
                    asm_exit_func(output);
                } else {
                    printf("can't reutrn ourside function\n");
                }
            }
        } else if (current_tok->type == WHILE) {


            fprintf(output, ".WHILE_%d:\n", while_count);

            stack_pop_asm(output, 't', 0);
            fprintf(output, "\tBEQZ t0, .ENDWHILE_%d\n", while_count);
            int_stack_push_val(while_stack, while_count);
            while_count++;

        } else if (current_tok->type == ENDWHILE) {

            int count = int_stack_pop_val(while_stack);
            fprintf(output, "\tJ .WHILE_%d\n", count);

            fprintf(output, ".ENDWHILE_%d:\n", count);


        } else if (current_tok->type == IF) {

            stack_pop_asm(output, 't', 0);
             fprintf(output, ".IF_%d:\n", if_count);
            fprintf(output, "\tBEQZ t0, .ELSE_%d\n", if_count);
            int_stack_push_val(if_stack, if_count);

             int_stack_push_val(else_stack, if_count);
            if_count++;

        } else if (current_tok->type == ELSE) {

            if (int_stack_count(else_stack) > 0) {
                int else_value = int_stack_pop_val(else_stack);
                 fprintf(output, "\tJ .ENDIF_%d\n", else_value);

                fprintf(output, ".ELSE_%d:\n", else_value);

            } else {
                printf("elseStack is empty\n");
            }
        } else if (current_tok->type == ENDIF) {

            if (int_stack_count(if_stack) > 0) {
                 int if_value = int_stack_pop_val(if_stack);
                if (int_stack_count(else_stack) > 0) {
                    int else_value = int_stack_pop_val(else_stack);
                    fprintf(output, ".ELSE_%d:\n", else_value);
                    fprintf(output, ".ENDIF_%d:\n", if_value);

                } else {
                    fprintf(output, ".ENDIF_%d:\n", if_value);

                }
            } else {
                printf(" ifStack  empty\n");
            }
        } else if (current_tok->type == GET_ARG) {
            i = current_tok->arg_no;
             stack_push_asm(output, 'a', i - 1);

        } else if (current_tok->type == SET_ARG) {
             i = current_tok->arg_no;
            stack_pop_asm(output, 'a', i - 1);

        } else {
            asm_generate(output, *current_tok);
        }
    }

     fclose(j_file);
     fclose(output);
     free(current_tok);

    int_stack_destroy(while_stack);
    int_stack_destroy(if_stack);
    int_stack_destroy(else_stack);

    return 0;
}

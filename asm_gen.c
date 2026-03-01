
#include "asm_gen.h"
#include <stdlib.h>
#include <string.h>


static void handle_literal(FILE *out, token tk);
static void handle_arithmetic(FILE *out, token tk);
static void handle_bitwise(FILE *out, token tk);
static void handle_ident(FILE *out, token tk);
static void handle_comparison(FILE *out, token tk);
static void handle_stackops(FILE *out, token tk);


void stack_push_asm(FILE *out, char reg, int offset) {
    fprintf(out, "addi sp, sp, -4\n");
    fprintf(out, "sw %c%d, 0(sp)\n", reg, offset);
}

void stack_pop_asm(FILE *out, char reg, int offset) {
    fprintf(out, "lw %c%d, 0(sp)\n", reg, offset);
    fprintf(out, "addi sp, sp, 4\n");
}

void asm_setup(FILE *out) {
    fprintf(out, ".text\n");
    fprintf(out, ".p2align 2\n");
}

void asm_enter_func(FILE *out) {
    fprintf(out, "addi sp, sp, -16\n");
    fprintf(out, "sw ra, 12(sp)\n");
    fprintf(out, "sw fp, 8(sp)\n");
    fprintf(out, "addi fp, sp, 16\n");
}

void asm_exit_func(FILE *out) {
    fprintf(out, "lw ra, 12(sp)\n");
    fprintf(out, "lw fp, 8(sp)\n");
    fprintf(out, "addi sp, sp, 16\n");
    fprintf(out, "ret\n");
}

static void handle_literal(FILE *out, token tk) {
    fprintf(out, "li t0, %d\n", tk.literal_value);
    stack_push_asm(out, 't', 0);
}

static void handle_arithmetic(FILE *out, token tk) {
    const char *op_str = NULL;
    switch (tk.type) {
        case PLUS:  op_str = "add"; break;
        case MINUS: op_str = "sub"; break;
        case MUL:   op_str = "mul"; break;
        case DIV:   op_str = "div"; break;
        case MOD:   op_str = "rem"; break;
        default:
            fprintf(stderr, "unknown op\n");
            exit(1);
    }

    stack_pop_asm(out, 't', 0); // a
    stack_pop_asm(out, 't', 1); // b
    fprintf(out, "%s t0, t0, t1\n", op_str);
    stack_push_asm(out, 't', 0);
}

static void handle_bitwise(FILE *out, token tk) {
    if (tk.type == AND || tk.type == OR) {
        stack_pop_asm(out, 't', 0);
        stack_pop_asm(out, 't', 1);
        if (tk.type == AND)
            fprintf(out, "and t0, t0, t1\n");
        else
            fprintf(out, "or t0, t0, t1\n");
        stack_push_asm(out, 't', 0);
    } else if (tk.type == NOT) {
        stack_pop_asm(out, 't', 0);
        fprintf(out, "xori t0, t0, -1\n");
        stack_push_asm(out, 't', 0);
    } else {
        fprintf(stderr, "invalid token\n");
        exit(1);
    }
}

static void handle_ident(FILE *out, token tk) {
    fprintf(out, "call %s\n", tk.str);
    stack_push_asm(out, 'a', 0);
}

static void handle_comparison(FILE *out, token tk) {
    stack_pop_asm(out, 't', 0);
    stack_pop_asm(out, 't', 1);

    fprintf(out, "sub t0, t0, t1\n");

    switch (tk.type) {
        case LT: 
            fprintf(out, "sltz t0, t0\n"); 
            break;
        case LE:
            fprintf(out, "sltz t1, t0\n");
            fprintf(out, "seqz t0, t0\n");
            fprintf(out, "or t0, t0, t1\n");
            break;
        case EQ:
            fprintf(out, "seqz t0, t0\n");
            break;
        case GE:
            fprintf(out, "sgtz t1, t0\n");
            fprintf(out, "seqz t0, t0\n");
            fprintf(out, "or t0, t0, t1\n");
            break;
        case GT:
            fprintf(out, "sgtz t0, t0\n");
            break;
        default:
            fprintf(stderr, "unknown op\n");
            exit(1);
    }
    stack_push_asm(out, 't', 0);
}

static void handle_stackops(FILE *out, token tk) {
    if (tk.type == DROP) {
        fprintf(out, "addi sp, sp, 4\n");
    } else if (tk.type == DUP) {
        stack_pop_asm(out, 't', 0);
        stack_push_asm(out, 't', 0);
        stack_push_asm(out, 't', 0);
    } else if (tk.type == SWAP) {
        stack_pop_asm(out, 't', 0);
        stack_pop_asm(out, 't', 1);
        stack_push_asm(out, 't', 0);
        stack_push_asm(out, 't', 1);
    } else if (tk.type == ROT) {
        stack_pop_asm(out, 't', 0);
        stack_pop_asm(out, 't', 1);
        stack_pop_asm(out, 't', 2);
        // rotate top three
        stack_push_asm(out, 't', 1);
        stack_push_asm(out, 't', 0);
        stack_push_asm(out, 't', 2);
    } else {
        fprintf(stderr, "invalid operation\n");
        exit(1);
    }
}

void asm_generate(FILE *out, token tk) {
    switch (tk.type) {
        case LITERAL:
            handle_literal(out, tk);
            break;
        case PLUS: case MINUS: case MUL: case DIV: case MOD:
            handle_arithmetic(out, tk);
            break;
        case AND: case OR: case NOT:
            handle_bitwise(out, tk);
            break;
        case IDENT:
            handle_ident(out, tk);
            break;
        case LT: case LE: case EQ: case GE: case GT:
            handle_comparison(out, tk);
            break;
        case DROP: case DUP: case SWAP: case ROT:
            handle_stackops(out, tk);
            break;
        default:

            fprintf(stderr, "unhandled token type %d\n", tk.type);

            break;
    }
}


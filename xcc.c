#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of argument\n");
        return 1;
    }

    char *p = argv[1];

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    long char_long = strtol(p, &p, 10);
    printf("    mov rax, %d\n", char_long);

    while (*p) {
        if (*p == '+') {
            p++;
            char_long = strtol(p, &p, 10);
            printf("    add rax, %ld\n", char_long);
        } else if (*p == '-') {
            p++;
            char_long = strtol(p, &p, 10);
            printf("    sub rax, %ld\n", char_long);
        } else {
            fprintf(stderr, "unexpected token: '%c'\n", *p);
            return 1;
        }
    }

    printf("    ret\n");
    return 0;
}
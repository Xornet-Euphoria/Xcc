#include "xcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of argument\n");
        return 1;
    }

    user_input = argv[1];
    // tokenize
    current_token = tokenize(argv[1]);

    // 抽象木の作成
    new_program();

    generate_asm();

    return 0;
}
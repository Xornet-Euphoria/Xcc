#include "xcc.h"

Node *code[100];
static int label_index = 0;

// 代入式で次に操作するアドレスを用意
void gen_lvar(Node *node) {
    if (node->kind != ND_LVAR) {
        simple_error("left hand side must be variable identifer");
    }

    printf("    mov rax, rbp\n");
    printf("    sub rax, %d\n", node->offset);
    printf("    push rax\n"); // スタックトップに指定変数の値を格納するアドレスが来る
}

static void gen(Node *node) {
    int current_label;
    current_label = label_index;
    label_index++;

    int f_length;
    char f_name[100];

    if (node->kind == ND_FUNC_DEF) {
        f_length = node->def_func->len;
        strncpy(f_name, node->def_func->name, f_length);
        f_name[f_length] = '\0';

        printf(".global %s\n", f_name);
        printf("%s:\n", f_name);

        // ローカル変数確保
        int var_count = 0;
        for (LVar *lvar = node->def_func->start; lvar; lvar = lvar->next) {
            var_count++;
        }

        printf("    push rbp\n");
        printf("    mov rbp, rsp\n");
        printf("    sub rsp, %d\n", var_count * 8);

        // 渡された引数の処理
        int arg_num = node->def_func->arg_num;

        for (int i = 1; i <= arg_num; i++) {
            printf("    mov rax, rbp\n");
            printf("    sub rax, %d\n", i * 8);

            switch (i) {
                case 1:
                    printf("    mov [rax], rdi\n");
                    break;
                case 2:
                    printf("    mov [rax], rsi\n");
                    break;
                case 3:
                    printf("    mov [rax], rdx\n");
                    break;
                case 4:
                    printf("    mov [rax], rcx\n");
                    break;
                case 5:
                    printf("    mov [rax], r8\n");
                    break;
                case 6:
                    printf("    mov [rax], r9\n");
                    break;
            }
        }

        gen(node->lhs);
        return;
    }

    if (node->kind == ND_FUNC_CALL) {
        f_length = node->func->len;
        strncpy(f_name, node->func->name, f_length);
        f_name[f_length] = '\0';

        // 引数を用意
        if (node->func->start != NULL) {
            Arg *current_arg = node->func->start;

            while (true) {
                gen(current_arg->value);
                printf("    pop rax\n");
                switch (current_arg->arg_num) {
                    case 1:
                        printf("    mov rdi, rax\n");
                        break;
                    case 2:
                        printf("    mov rsi, rax\n");
                        break;
                    case 3:
                        printf("    mov rdx, rax\n");
                        break;
                    case 4:
                        printf("    mov rcx, rax\n");
                        break;
                    case 5:
                        printf("    mov r8, rax\n");
                        break;
                    case 6:
                        printf("    mov r9, rax\n");
                        break;
                }

                if (current_arg->next == NULL) {
                    break;
                }

                current_arg = current_arg->next;
            }
        }

        printf("    call %s\n", f_name);
        printf("    push rax\n");
        return;
    }

    switch (node->kind) {
        case ND_NUM:
            printf("    push %d\n", node->val);
            return;
        // 代入左辺以外で変数が示された場合
        case ND_LVAR:
            gen_lvar(node);
            printf("    pop rax\n");
            printf("    mov rax, [rax]\n"); // raxで指定したアドレスの値をraxに再代入する
            printf("    push rax\n");
            return;
        case ND_ASSIGN:
            gen_lvar(node->lhs); // スタックトップから2番目に変数のアドレス
            
            gen(node->rhs); // スタックトップに右辺計算結果

            printf("    pop rdi\n"); // 右辺計算結果
            printf("    pop rax\n"); // 代入先アドレス
            printf("    mov [rax], rdi\n");
            printf("    push rdi\n"); // 二重代入(a=b=2)を許すためにrdiを再利用する
            return;
        case ND_IF:
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            if (node->rhs != NULL) {
                // else節が存在する場合
                printf("    je .Lelse%d\n", current_label);
                gen(node->lhs);
                printf("    jmp .Lend%d\n", current_label);
                printf(".Lelse%d:\n", current_label);
                gen(node->rhs);
            } else {
                printf("    je .Lend%d\n", current_label);
                gen(node->lhs);
            }
            printf(".Lend%d:\n", current_label);
            return;
        case ND_WHILE:
            printf(".Lbegin%d:\n", current_label);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", current_label);
            gen(node->lhs);
            printf("    jmp .Lbegin%d\n", current_label);
            printf(".Lend%d:\n", current_label);
            return;
        case ND_FOR:
            gen(node->init);
            printf(".Lbegin%d:\n", current_label);
            gen(node->cond);
            printf("    pop rax\n");
            printf("    cmp rax, 0\n");
            printf("    je .Lend%d\n", current_label);
            gen(node->rhs);
            gen(node->lhs);
            printf("    jmp .Lbegin%d\n", current_label);
            printf(".Lend%d:\n", current_label);
            return;
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
            return;
        case ND_BLOCK:
            for (int i = 0; node->block[i] != NULL; i++) {
                gen(node->block[i]);
                if (node->block[i + 1] == NULL) {
                    break;
                }
                printf("    pop rax\n");
            }
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    // 左から演算をしていくことからraxの値をrdiの値を使って操作していくことにする
    printf("    pop rdi\n"); // rhs
    printf("    pop rax\n"); // lhs

    switch (node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            break;
        case ND_MOD:
            printf("    cqo\n");
            printf("    idiv rdi\n");
            printf("    mov rax, rdx\n");
            break;
        case ND_LT:
            printf("    cmp rax, rdi\n");
            printf("    setl al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_LEQ:
            printf("    cmp rax, rdi\n");
            printf("    setle al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_EQ:
            printf("    cmp rax, rdi\n");
            printf("    sete al\n");
            printf("    movzb rax, al\n");
            break;
        case ND_NEQ:
            printf("    cmp rax, rdi\n");
            printf("    setne al\n");
            printf("    movzb rax, al\n");
            break;
    }

    printf("    push rax\n");
}

void generate_asm() {
    printf(".intel_syntax noprefix\n");

    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }
}
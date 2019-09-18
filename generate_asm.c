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
        case ND_RETURN:
            gen(node->lhs);
            printf("    pop rax\n");
            printf("    mov rsp, rbp\n");
            printf("    pop rbp\n");
            printf("    ret\n");
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
    printf(".global main\n");
    printf("main:\n");

    // ローカル変数確保
    int var_count = 0;
    for (LVar *lvar = local_var; lvar; lvar = lvar->next) {
        var_count++;
    }
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, %d\n", var_count * 8);

    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);

        // 各処理において最後の計算結果がスタックトップにあるはずなのでそれをpopする
        // したがって任意のcode処理の結果は必ずraxに格納される
        printf("    pop rax\n");
    }

    printf("    mov rsp, rbp\n"); // スタックを元に戻す
    printf("    pop rbp\n");
    printf("    ret\n");
}
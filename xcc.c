#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類を定義
typedef enum {
    TK_RESERVED,
    TK_NUM,      // 数字
    TK_EOF,      // 終端
} TokenKind;

// Token構造体を単にTokenとして定義
typedef struct Token Token;

// Token構造体
// これを複数繋げていくことでトークナイズする
struct Token {
    TokenKind kind; // トークン種
    Token *next;    // 次のトークン
    int val;        // 数字ならその値
    char *str;      // トークン文字
};

typedef enum {
    ND_ADD,
    ND_SUB,
    ND_NUM,
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    int val;
};

// 現時点で注目しているトークン、これを進めていくことで順にトークンを処理していく
Token *current_token;

// ユーザー入力(mainでargv[1]に保存されているいるもの)
char *user_input;

// エラー出力
void error(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    // 現在見ている文字のアドレスからuser_input始点アドレスを引くことで何文字目かを特定
    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// トークン種が期待通りだったときに次のトークンへ進める処理群
// 単なる文字判定
bool consume(char op) {
    if (current_token->kind != TK_RESERVED || current_token->str[0] != op) {
        return false;
    }
    // 次へ進める
    current_token = current_token->next;
    return true;
}

// 期待文字列でなければエラー
void expect(char op) {
    if (current_token->kind != TK_RESERVED || current_token->str[0] != op) {
        error(current_token->str, "this token is not '%c'", op);
    }

    current_token = current_token->next;
}

// 数値を期待しそれを返す
int expect_number() {
    if (current_token->kind != TK_NUM) {
        error(current_token->str, "this token is not a number");
    }

    int val = current_token->val;
    current_token = current_token->next;
    return val;
}

bool at_eof() {
    return current_token->kind == TK_EOF;
}

Node *new_expr();
Node *new_mul();
Node *new_primary();

// 新しいノード(但し数字を除く)
Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *new_nd = calloc(1, sizeof(Node));
    new_nd->kind = kind;
    new_nd->lhs = lhs;
    new_nd->rhs = rhs;
    return new_nd;
}

Node *new_node_num(int val) {
    Node *new_nd = calloc(1, sizeof(Node));
    new_nd->kind = ND_NUM;
    new_nd->val = val;
    return new_nd;
}

Node *new_expr() {
    Node *node = new_mul();

    while (true) {
        if (consume('+')) {
            node = new_node(ND_ADD, node, new_mul());
        } else if (consume('-')) {
            node = new_node(ND_SUB, node, new_mul());
        } else {
            return node;
        }
    }
}

Node *new_mul() {
    Node *node = new_primary();

    while (true) {
        return node;
    }
}

Node *new_primary() {
    return new_node_num(expect_number());
}

// *curに続く新しいトークンを種類と文字を指定して生成し、それを返す
Token *new_token(TokenKind kind, Token *cur, char *str) {
    // Tokenと同じサイズのメモリを確保
    Token *new_tk = calloc(1, sizeof(Token));
    new_tk->kind = kind;
    new_tk->str = str;
    cur->next = new_tk;
    return new_tk;
}

// 入力をトークナイズしそれの先頭を返す
Token *tokenize(char *p) {
    // 先頭の定義
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // 空白はトークンとして追加しない
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p);
            p++;
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error(p, "Can't tokenize this token");
    }

    new_token(TK_EOF, cur, p);
    // 開始地点を返す
    return head.next;
}

void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
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
    }

    printf("    push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of argument\n");
        return 1;
    }

    user_input = argv[1];
    // tokenize
    current_token = tokenize(argv[1]);
    // 抽象木の作成
    Node *node = new_expr();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    gen(node);

    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
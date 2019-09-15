#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類を定義
typedef enum {
    TK_RESERVED, // 記号(+,-)
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

// 現時点で注目しているトークン、これを進めていくことで順にトークンを処理していく
Token *current_token;

// エラー出力
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
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
        error("this token is not '%c'", op);
    }

    current_token = current_token->next;
}

// 数値を期待しそれを返す
int expect_number() {
    if (current_token->kind != TK_NUM) {
        error("this token is not a number");
    }

    int val = current_token->val;
    current_token = current_token->next;
    return val;
}

bool at_eof() {
    return current_token->kind == TK_EOF;
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

        error("Can't tokenize this token");
    }

    new_token(TK_EOF, cur, p);
    // 開始地点を返す
    return head.next;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "invalid number of argument\n");
        return 1;
    }

    // tokenize
    current_token = tokenize(argv[1]);

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の頭には数字が来ることを期待する
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf("    sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}
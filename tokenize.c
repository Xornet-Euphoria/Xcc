#include "xcc.h"

char *user_input;
Token *current_token;

// トークン種が期待通りだったときに次のトークンへ進める処理群
// 単なる文字判定
bool consume(char *op) {
    if (current_token->kind != TK_RESERVED || 
        current_token->len != strlen(op)   || 
        strncmp(current_token->str, op, current_token->len) != 0) {
        return false;
    }
    // 次へ進める
    current_token = current_token->next;
    return true;
}

// 期待文字列でなければエラー
void expect(char *op) {
    if (current_token->kind != TK_RESERVED || 
        current_token->len != strlen(op)   || 
        strncmp(current_token->str, op, current_token->len) != 0) {
        error(current_token->str, "this token is not '%s'", op);
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

static Token *new_token(TokenKind kind, Token *cur, char *str) {
    // Tokenと同じサイズのメモリを確保
    Token *new_tk = calloc(1, sizeof(Token));
    new_tk->kind = kind;
    new_tk->str = str;
    cur->next = new_tk;
    return new_tk;
}

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

        if (strncmp(p, "==", 2) == 0 ||
            strncmp(p, "!=", 2) == 0 ||
            strncmp(p, "<=", 2) == 0 ||
            strncmp(p, ">=", 2) == 0) {
                cur = new_token(TK_RESERVED, cur, p);
                cur->len = 2;
                p += 2;
                continue;
            }

        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 1;
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
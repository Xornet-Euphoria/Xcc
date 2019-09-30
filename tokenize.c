#include "xcc.h"

char *user_input;
Token *current_token;

// エラー関連
void simple_error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // print pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

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

// 変数識別子を期待
Token *consume_ident() {
    if (current_token->kind != TK_IDENT) {
        error(current_token->str, "this token is not identifier");
    }

    Token *ret_token = current_token;
    current_token = current_token->next;
    return ret_token;
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

static bool strictstrcmp(char *haystack, char *needle) {
    int length = strlen(needle);
    return (strncmp(haystack, needle, length) == 0 && !isalnum(haystack[length]));
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

        if (strictstrcmp(p, "return")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 6;
            p += 6;
            continue;
        }

        if (strictstrcmp(p, "while")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 5;
            p += 5;
            continue;
        }

        if (strictstrcmp(p, "else")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 4;
            p += 4;
            continue;
        }

        if (strictstrcmp(p, "for")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 3;
            p += 3;
            continue;
        }

        if (strictstrcmp(p, "if")) {
            cur = new_token(TK_RESERVED, cur, p);
            cur->len = 2;
            p += 2;
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

        if (strchr("+-*/%(){}<>=,;", *p)) {
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

        if (isalpha(*p)) {
            char *q = p;
            p++;
            int length = 1;
            while (isalnum(*p)) {
                p++;
                length++;
            }
            cur = new_token(TK_IDENT, cur, q);
            cur->len = length;
            continue;
        }

        error(p, "Can't tokenize this token");
    }

    new_token(TK_EOF, cur, p);
    // 開始地点を返す
    return head.next;
}
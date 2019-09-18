#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// token
// トークンの種類を定義
typedef enum {
    // returnもifもreservedでまとめられるのでは?
    TK_RESERVED, // 予約語
    TK_IDENT,    // 識別子(変数とか)
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
    char *str;      // トークン文字列
    int len;        // トークン長(==, <=等に対応))
};

// node
typedef enum {
    ND_ADD,
    ND_SUB,
    ND_MUL,
    ND_DIV,
    ND_MOD,    // modulo (%)
    ND_NUM,
    ND_EQ,     // equal(==)
    ND_NEQ,    // not equal(!=)
    ND_LT,     // less than(<)
    ND_LEQ,    // less or equal than(<=)
    ND_ASSIGN, // assignment(=)
    ND_LVAR,   // local variable
    ND_RETURN, // return
    ND_IF,     // if
    ND_WHILE,  // while
    ND_FOR,    // for
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind;
    Node *lhs;
    Node *rhs;
    Node *cond; // ループやジャンプの条件節
    Node *init; // forの初期化文
    int val;
    int offset; // ローカル変数のスタック上での位置(rbp - x)
};

typedef struct LVar LVar;

struct LVar {
    LVar *next;
    char *name;
    int len;
    int offset;
};

// variables
// 現時点で注目しているトークン、これを進めていくことで順にトークンを処理していく
extern Token *current_token;

// ユーザー入力(mainでargv[1]に保存されているいるもの)
extern char *user_input;

// コード
extern Node *code[100];

// 現在見ているローカル変数
LVar *local_var;

// prototype declaration
void simple_error(char *fmt, ...);
void error(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool consume_type(TokenKind);
Token *consume_ident();
bool next_check(char *op);
bool at_eof();
Token *tokenize(char *p);
void new_program();
void generate_asm();
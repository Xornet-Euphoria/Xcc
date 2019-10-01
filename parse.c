#include "xcc.h"

Node *code[100];
LVar *local_var;
static int current_offset;

static Node *new_function();
static Node *new_stmt();
static Node *new_expr();
static Node *new_assign();
static Node *new_equality();
static Node *new_relational();
static Node *new_add();
static Node *new_mul();
static Node *new_unary();
static Node *new_primary();

// 新しいノード(但し数字を除く)
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *new_nd = calloc(1, sizeof(Node));
    new_nd->kind = kind;
    new_nd->lhs = lhs;
    new_nd->rhs = rhs;
    return new_nd;
}

static Node *new_node_num(int val) {
    Node *new_nd = calloc(1, sizeof(Node));
    new_nd->kind = ND_NUM;
    new_nd->val = val;
    return new_nd;
}

// 名前で変数検索
static LVar *find_lvar(Token *tok) {
    for (LVar *var = local_var; var; var = var->next) {
        if (var->len == tok->len && strncmp(tok->str, var->name, var->len) == 0) {
            return var;
        }
    }

    return NULL;
}

void new_program() {
    int i = 0;
    while (!at_eof()) {
        current_offset = 0;
        local_var = NULL;
        code[i] = new_function();
        i++;
    }
    // 終端
    code[i] = NULL;
}

// function
static Node *new_function() {
    Node *node;
    
    Token *tk = consume_ident();
    expect("(");
    node = calloc(1, sizeof(Node));
    node->kind = ND_FUNC_DEF;
    DefFunc *def_func = calloc(1, sizeof(DefFunc));
    def_func->name = tk->str;
    def_func->len = tk->len;
    node->def_func = def_func;

    int arg_num = 0;

    while (!consume(")")) {
        Token *tk = consume_ident();

        // 引数の数
        LVar *lvar = find_lvar(tk);
    
        // todo: 関数定義で同じ名前の引数が出たときの対処
        if (lvar == NULL) {
            arg_num++;
            lvar = calloc(1, sizeof(LVar));
            lvar->next = local_var;
            lvar->name = tk->str;
            lvar->len = tk->len;
            lvar->offset = current_offset + 8;
            current_offset = lvar->offset;
            local_var = lvar;
        }

        if (!consume(",")) {
            expect(")");
            break;
        }
    }

    node->lhs = new_stmt();
    def_func->start = local_var;
    def_func->arg_num = arg_num;

    return node;
}

// statement
static Node *new_stmt() {
    Node *node;

    // if
    if (consume("if")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_IF;
        node->cond = new_expr();
        expect(")");
        
        node->lhs = new_stmt();

        if (consume("else")) {
            node->rhs = new_stmt();
        } else {
            node->rhs = NULL;
        }

        return node;
    }

    // while
    if (consume("while")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_WHILE;
        node->cond = new_expr();
        expect(")");

        node->lhs = new_stmt();

        return node;
    }

    // for, まずは必ず初期化や条件が存在している場合
    if (consume("for")) {
        expect("(");
        node = calloc(1, sizeof(Node));
        node->kind = ND_FOR;
        node->init = new_expr();
        expect(";");
        node->cond = new_expr();
        expect(";");
        node->lhs = new_expr();
        expect(")");
        node->rhs = new_stmt();

        return node;
    }

    if (consume("{")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_BLOCK;
        int index = 0;
        while(!consume("}")) {
            node->block[index] = new_stmt();
            index++;
        }

        // 終端
        node->block[index] = NULL;

        return node;
    }

    if (consume("return")) {
        node = calloc(1, sizeof(Node));
        node->kind = ND_RETURN;
        node->lhs = new_expr(); // 1引数は左辺とみなす
    } else {
        node = new_expr();
    }

    expect(";");

    return node;
}

static Node *new_expr() {
    return new_assign();
}

static Node *new_assign() {
    Node *node = new_equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, new_assign());
    }

    return node;
}

static Node *new_equality() {
    Node *node = new_relational();
    
    while (true) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, new_add());
        } else if (consume("!=")) {
            node = new_node(ND_NEQ, node, new_add());
        } else {
            return node;
        }
    }

    return node;
}

static Node *new_relational() {
    Node *node = new_add();

    while (true) {
        if (consume("<")) {
            node = new_node(ND_LT, node, new_add());
        } else if (consume(">")) {
            // a > b と b < a は同値
            node = new_node(ND_LT, new_add(), node);
        } else if (consume("<=")) {
            node = new_node(ND_LEQ, node, new_add());
        } else if (consume(">=")) {
            node = new_node(ND_LEQ, new_add(), node);
        } else {
            return node;
        }
    }
}

static Node *new_add() {
    Node *node = new_mul();

    while (true) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, new_mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, new_mul());
        } else {
            return node;
        }
    }
}

static Node *new_mul() {
    Node *node = new_unary();

    while (true) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, new_unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, new_unary());
        } else if (consume("%")) {
            node = new_node(ND_MOD, node, new_unary());
        } else {
            return node;
        }
    }
}

static Node *new_unary() {
    // +x = x と扱う
    if (consume("+")) {
        return new_primary();
    }

    // -x = (0 - x) と扱う
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), new_primary());
    }

    return new_primary();
}

static Node *new_primary() {
    if (consume("(")) {
        Node *node = new_expr();
        // 括弧が閉じられているかの確認
        expect(")");
        return node;
    }

    if (current_token->kind == TK_NUM) {
        return new_node_num(expect_number());
    }

    // todo: new_node_identとか作った方が楽かもしれない
    Node *new_nd = calloc(1, sizeof(Node));

    Token *tk = consume_ident();
    
    // 関数呼び出し
    if (consume("(")) {
        new_nd->kind = ND_FUNC_CALL;
        Func *new_func = calloc(1, sizeof(Func));
        new_nd->func = new_func;
        new_func->name = tk->str;
        new_func->len = tk->len;
        new_func->start = NULL;
        int arg_num = 0;

        Arg *previous_arg;

        while (!consume(")")) {
            arg_num++;
            
            Node *arg_node = new_equality();
            Arg *new_arg = calloc(1, sizeof(Arg));
            new_arg->value = arg_node;

            new_arg->arg_num = arg_num;

            if (new_func->start == NULL) {
                new_func->start = new_arg;
            } else {
                previous_arg->next = new_arg;
            }

            previous_arg = new_arg;

            if (!consume(",")) {
                new_arg->next = NULL;
                expect(")");
                break;
            }
        }

        return new_nd;
    }

    // 変数
    new_nd->kind = ND_LVAR;
    LVar *lvar = find_lvar(tk);
    
    if (lvar == NULL) {
        lvar = calloc(1, sizeof(LVar));
        lvar->next = local_var;
        lvar->name = tk->str;
        lvar->len = tk->len;
        lvar->offset = current_offset + 8;
        current_offset = lvar->offset;
        local_var = lvar;
    }

    new_nd->offset = lvar->offset;
    return new_nd;
}
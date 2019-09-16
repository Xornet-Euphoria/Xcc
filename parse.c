#include "xcc.h"

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

Node *new_expr() {
    return new_equality();
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

    return new_node_num(expect_number());
}
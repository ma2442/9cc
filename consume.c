#include "9cc.h"

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    switch (token->kind) {
        case TK_RESERVED:
        case TK_SIZEOF:
        case TK_RETURN:
        case TK_CTRL:
        case TK_STRUCT:
            break;
        default:
            return false;
    }
    if (strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_if_kind_is(TokenKind tk) {
    if (token->kind != tk) {
        return NULL;
    }
    Token *this = token;
    token = token->next;
    return this;
}
Token *consume_numeric() {
    Token *tok = consume_if_kind_is(TK_CHAR);
    if (tok) return tok;
    return consume_if_kind_is(TK_NUM);
}
Token *consume_str() { return consume_if_kind_is(TK_STR); }
Token *consume_type() {
    Token *tok = consume_if_kind_is(TK_STRUCT);
    if (tok) return tok;
    tok = consume_if_kind_is(TK_TYPE);
    if (tok) return tok;
    return consume_if_kind_is(TK_ENUM);
}
Token *consume_ident() { return consume_if_kind_is(TK_IDENT); }

int consume_incdec() {
    if (consume("++")) return ND_ADD;
    if (consume("--")) return ND_SUB;
    return -1;
}

//複合代入
int consume_compo_assign() {
    if (consume("+=")) return ND_ADD;
    if (consume("-=")) return ND_SUB;
    if (consume("*=")) return ND_MUL;
    if (consume("/=")) return ND_DIV;
    if (consume("%=")) return ND_MOD;
    return -1;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if ((token->kind != TK_RESERVED && token->kind != TK_CTRL) ||
        strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
    return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_numeric() {
    if (token->kind != TK_NUM && token->kind != TK_CHAR) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

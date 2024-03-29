#include "9cc.h"

// プリプロセッサ用 for,while, int, signed などもひっくるめて識別子と認識
Token *consume_identpp() {
    if (!token) return NULL;
    switch (token->kind) {
        case TK_CHAR:
        case TK_NUM:
        case TK_STR:
        case TK_DEFAULT_PATH:
        case TK_NUMSUFFIX:
        case TK_RESERVED:
            return NULL;
    }
    Token *this = token;
    token = token->next;
    return this;
}

bool current_is(char *op) {
    if (!token) return false;
    if (!eqtokstr(token, op)) return false;
    return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    if (!token) return false;
    switch (token->kind) {
        case TK_STR:
        case TK_CHAR:
        case TK_NUM:
            return false;
    }
    if (!eqtokstr(token, op)) return false;
    token = token->next;
    return true;
}

Token *consume_if_kind_is(TokenKind tk) {
    if (!token) return NULL;
    if (token->kind != tk) return NULL;
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
Token *consume_typeq() {
    Token *tok = consume_if_kind_is(TK_TYPEQ_SIGN);
    if (tok) return tok;
    return consume_if_kind_is(TK_TYPEQ_LENGTH);
}
Token *consume_typecore() {
    Token *tok = consume_if_kind_is(TK_STRUCT);
    if (tok) return tok;
    tok = consume_if_kind_is(TK_UNION);
    if (tok) return tok;
    tok = consume_if_kind_is(TK_ENUM);
    if (tok) return tok;
    return consume_if_kind_is(TK_TYPE);
}
Token *consume_ident() { return consume_if_kind_is(TK_IDENT); }
Token *consume_numsuffix() { return consume_if_kind_is(TK_NUMSUFFIX); }

Token *consume_tag_without_def(TokenKind kind) {
    Token *save = token;
    Token *idt = NULL;
    if (consume_if_kind_is(kind)) idt = consume_ident();
    if (idt && !consume("{")) return idt;
    token = save;
    return NULL;
}

int consume_incdec() {
    if (consume("++")) return ND_ADD;
    if (consume("--")) return ND_SUB;
    return -1;
}

//複合代入
int consume_compo_assign() {
    if (consume("<<=")) return ND_BIT_SHIFT_L;
    if (consume(">>=")) return ND_BIT_SHIFT_R;
    if (consume("|=")) return ND_BIT_OR;
    if (consume("^=")) return ND_BIT_XOR;
    if (consume("&=")) return ND_BIT_AND;
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
        !eqtokstr(token, op)) {
        error_at2(token->str, ERRNO_EXPECT, op);
    }
    token = token->next;
    return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_numeric() {
    if (token->kind == TK_NUM || token->kind == TK_CHAR) {
        int val = token->val;
        token = token->next;
        return val;
    } else if (token->kind == TK_IDENT) {
        Def *dsym = fit_def(token, DK_ENUMCONST);
        if (dsym) {
            token = token->next;
            return dsym->cst->val;
        }
    }
    error_at(token->str, ERRNO_PARSE_NUM);
}

bool at_eof() { return token->kind == TK_EOF; }

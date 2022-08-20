#include "9cc.h"

bool sametok(Token *tok1, Token *tok2) {
    if (!tok1 || !tok2) return false;
    if (tok1->len != tok2->len) return false;
    if (strncmp(tok1->str, tok2->str, tok1->len) != MATCH) return false;
    return true;
}

// tokenと文字列の一致を調べる
bool eqtokstr(Token *tok, char *str) {
    if (!tok) return false;
    if (tok->len != strlen(str)) return false;
    if (strncmp(tok->str, str, tok->len) != MATCH) return false;
    return true;
}

// トークン列t1の後ろにt2を連結する
// t1にEOFが含まれていればそれを消してt2につなげる
Token *concat_tokens(Token *t1, Token *t2) {
    Token head;
    Token *cur = &head;
    cur->next = t1;
    while (cur->next && cur->next->kind != TK_EOF) {
        cur = cur->next;
    }
    cur->next = t2;
    return head.next;
}
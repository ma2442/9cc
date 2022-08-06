#include "9cc.h"

bool sametok(Token *tok1, Token *tok2) {
    if (!tok1 || !tok2) return false;
    if (tok1->len != tok2->len) return false;
    if (strncmp(tok1->str, tok2->str, tok1->len)) return false;
    return true;
}

// tokenと文字列の一致を調べる
bool eqtokstr(Token *tok, char *str) {
    if (!tok) return false;
    if (tok->len != strlen(str)) return false;
    if (strncmp(tok->str, str, tok->len)) return false;
    return true;
}

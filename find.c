#include "9cc.h"

void error_undef(Token *tok, DefKind kind) {
    if (kind == DK_VAR)
        error_at(tok->str, "未定義の変数です");
    else if (kind == DK_STRUCT)
        error_at(tok->str, "未定義の構造体です");
    else if (kind == DK_ENUM)
        error_at(tok->str, "未定義の列挙体です");
    // else if (kind == DK_ENUMCONST)
    //     error_at(tok->str, "未定義の定数です");
    // else if (kind == DK_FUNC)
    // error_at(tok->str, "未定義の関数です");
}

// 変数･関数･構造体･列挙体のいずれかを名前で検索する。
// 見つからなかった場合はNULLを返す。
Def *find_def(Token *tok, DefKind kind) {
    if (kind == DK_ENUMCONST) return find_enumconst(tok);
    Def *d = NULL;
    if (kind == DK_VAR)
        d = def[nest]->vars;
    else if (kind == DK_FUNC)
        d = def[nest]->funcs;
    else if (kind == DK_STRUCT)
        d = def[nest]->structs;
    else if (kind == DK_ENUM)
        d = def[nest]->enums;
    else if (kind == DK_TYPE)
        d = def[nest]->typdefs;
    for (; d && d->tok; d = d->next) {
        if (d->tok->len == tok->len &&
            !memcmp(d->tok->str, tok->str, tok->len)) {
            return d;
        }
    }
    return NULL;
}

// 列挙体定数を名前で検索する。 見つからなかった場合はNULLを返す。
Def *find_enumconst(Token *tok) {
    for (Def *d = def[nest]->enums; d; d = d->next) {
        for (Def *cst = d->enm->consts; cst && cst->tok; cst = cst->next) {
            if (cst->tok->len == tok->len &&
                !memcmp(cst->tok->str, tok->str, tok->len)) {
                return cst;
            }
        }
    }
    return NULL;
}

// スコープ内で定義済みの変数を検索
Def *fit_def_noerr(Token *tok, DefKind kind) {
    int store = nest;
    while (nest >= 0) {
        Def *d = find_def(tok, kind);
        if (d) {
            nest = store;
            return d;
        }
        nest--;
    }
    nest = store;
    return NULL;
}

// スコープ内で定義済みの変数を検索。なければエラー
Def *fit_def(Token *tok, DefKind kind) {
    Def *dfit = fit_def_noerr(tok, kind);
    if(dfit) return dfit;
    error_undef(tok, kind);
    return NULL;
}

// 関数、変数、定数、型の名前が定義可能か
bool can_def_symbol(Token *sym) {
    if (!find_def(sym, DK_VAR) && !find_def(sym, DK_ENUMCONST) &&
        !find_def(sym, DK_FUNC) && !find_def(sym, DK_TYPE))
        return true;
    error_at(sym->str, "定義済みのシンボルです");
    return false;
}

// 構造体、列挙体のタグが定義可能か
bool can_def_tag(Token *tag) {
    if (!find_def(tag, DK_ENUM) && !find_def(tag, DK_STRUCT)) return true;
    error_at(tag->str, "定義済みのタグです");
    return false;
}

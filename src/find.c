#include "9cc.h"

void error_undef(Token *tok, DefKind kind) {
    if (kind == DK_VAR)
        error_at(tok->str, ERRNO_FIT_VAR);
    else if (kind == DK_STRUCT)
        error_at(tok->str, ERRNO_FIT_STRUCT);
    else if (kind == DK_UNION)
        error_at(tok->str, ERRNO_FIT_UNION);
    else if (kind == DK_ENUM)
        error_at(tok->str, ERRNO_FIT_ENUM);
    // else if (kind == DK_ENUMCONST)
    //     error_at(tok->str, ERRNO_ENUMCONST);
    else if (kind == DK_FUNC)
        error_at(tok->str, ERRNO_FIT_FUNC);
}

// 変数･関数･構造体･列挙体のいずれかを名前で検索する。
// 見つからなかった場合はNULLを返す。
Def *find_def(Token *tok, DefKind kind) {
    if (kind == DK_ENUMCONST) return find_enumconst(tok);
    Def *d = NULL;
    if (kind == DK_VAR)
        d = def[nest]->dvars;
    else if (kind == DK_FUNC)
        d = def[nest]->dfns;
    else if (kind == DK_STRUCT)
        d = def[nest]->dstcs;
    else if (kind == DK_UNION)
        d = def[nest]->dunis;
    else if (kind == DK_ENUM)
        d = def[nest]->denms;
    else if (kind == DK_TYPE)
        d = def[nest]->dtypdefs;
    for (; d && d->next; d = d->next) {
        if (sametok(d->tok, tok)) return d;
    }
    return NULL;
}

// 列挙子を名前で検索する。 見つからなかった場合はNULLを返す。
Def *find_enumconst(Token *tok) {
    for (Def *denm = def[nest]->denms; denm; denm = denm->next) {
        for (Def *dcst = denm->enm->dconsts; dcst; dcst = dcst->next) {
            if (!dcst->tok) continue;
            if (sametok(dcst->tok, tok)) return dcst;
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
    if (dfit) return dfit;
    error_undef(tok, kind);
    return NULL;
}

// 関数、変数、定数、型の名前が定義可能か
bool can_def_symbol(Token *sym) {
    if (!find_def(sym, DK_VAR) && !find_def(sym, DK_ENUMCONST) &&
        !find_def(sym, DK_FUNC) && !find_def(sym, DK_TYPE))
        return true;
    error_at(sym->str, ERRNO_DEF_SYMBOL);
    return false;
}

// 構造体、共用体、列挙体のタグが定義可能か
bool can_def_tag(Token *tag) {
    if (!find_def(tag, DK_ENUM) && !find_def(tag, DK_STRUCT) &&
        !find_def(tag, DK_UNION))
        return true;
    error_at(tag->str, ERRNO_DEF_TAG);
    return false;
}

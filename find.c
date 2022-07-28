#include "9cc.h"

// 変数を名前で検索する。 見つからなかった場合はNULLを返す。
Var *find_var(Token *tok) {
    for (Var *var = def[nest]->vars; var != NULL; var = var->next) {
        if (var->len == tok->len && !memcmp(var->name, tok->str, tok->len)) {
            return var;
        }
    }
    return NULL;
}

// 関数を名前で検索する。 見つからなかった場合はNULLを返す。
Func *find_func(Token *tok) {
    for (Func *fn = def[nest]->funcs; fn != NULL; fn = fn->next) {
        if (fn->len == tok->len && !memcmp(fn->name, tok->str, tok->len)) {
            return fn;
        }
    }
    return NULL;
}

// 構造体を名前で検索する。 見つからなかった場合はNULLを返す。
Struct *find_struct(Token *tok) {
    for (Struct *stc = def[nest]->structs; stc != NULL; stc = stc->next) {
        if (stc->len == tok->len && !memcmp(stc->name, tok->str, tok->len)) {
            return stc;
        }
    }
    return NULL;
}

// 列挙体を名前で検索する。 見つからなかった場合はNULLを返す。
Enum *find_enum(Token *tok) {
    for (Enum *enm = def[nest]->enums; enm != NULL; enm = enm->next) {
        if (enm->len == tok->len && !memcmp(enm->name, tok->str, tok->len)) {
            return enm;
        }
    }
    return NULL;
}

// 列挙体定数を名前で検索する。 見つからなかった場合はNULLを返す。
EnumConst *find_enumconst(Token *tok) {
    for (Enum *enm = def[nest]->enums; enm != NULL; enm = enm->next) {
        for (EnumConst *cst = enm->consts; cst != NULL; cst = cst->next) {
            if (cst->len == tok->len &&
                !memcmp(cst->name, tok->str, tok->len)) {
                return cst;
            }
        }
    }
    return NULL;
}

// スコープ内で定義済みの変数を検索。なければエラー
Var *fit_var(Token *tok) {
    int store = nest;
    while (nest >= 0) {
        Var *var = find_var(tok);
        if (var) {
            nest = store;
            return var;
        }
        nest--;
    }
    error_at(tok->str, "未定義の変数です");
    return NULL;
}

// スコープ内で定義済みの関数を検索。なければエラー
Func *fit_func(Token *tok) {
    int store = nest;
    while (nest >= 0) {
        Func *fn = find_func(tok);
        if (fn) {
            nest = store;
            return fn;
        }
        nest--;
    }
    // error_at(tok->str, "未定義の関数です");
    nest = store;
    return NULL;
}

// スコープ内で定義済みの構造体を検索。なければエラー
Struct *fit_struct(Token *tag) {
    int store = nest;
    while (nest >= 0) {
        Struct *stc = find_struct(tag);
        if (stc) {
            nest = store;
            return stc;
        }
        nest--;
    }
    error_at(tag->str, "未定義の構造体です");
    return NULL;
}

// スコープ内で定義済みの列挙体を検索。なければエラー
Enum *fit_enum(Token *tag) {
    int store = nest;
    while (nest >= 0) {
        Enum *enm = find_enum(tag);
        if (enm) {
            nest = store;
            return enm;
        }
        nest--;
    }
    error_at(tag->str, "未定義の列挙体です");
    return NULL;
}

// スコープ内で定義済みの列挙体定数を検索。なければNULLを返す。
EnumConst *fit_enumconst(Token *tag) {
    int store = nest;
    while (nest >= 0) {
        EnumConst *cst = find_enumconst(tag);
        if (cst) {
            nest = store;
            return cst;
        }
        nest--;
    }
    error_at(tag->str, "未定義の定数です");
    return NULL;
}

// スコープ内で定義済みのシンボルを検索。なければエラー
Symbol *fit_symbol(Token *tok) {
    Symbol *sym = calloc(1, sizeof(Symbol));
    int store = nest;
    while (nest >= 0) {
        sym->enumconst = find_enumconst(tok);
        if (sym->enumconst) break;
        sym->var = find_var(tok);
        if (sym->var) break;
        sym->func = find_func(tok);
        if (sym->func) break;
        nest--;
    }
    if (nest >= 0) {
        nest = store;
        return sym;
    }
    free(sym);
    error_at(tok->str, "未定義のシンボルです");
    return NULL;
}

// 関数、変数、定数の名前が定義可能か
bool can_def_symbol(Token *sym) {
    if (!find_var(sym) && !find_enumconst(sym) && !find_func(sym)) return true;
    error_at(sym->str, "定義済みのシンボルです");
    return false;
}

// 構造体、列挙体のタグが定義可能か
bool can_def_tag(Token *tag) {
    if (!find_enum(tag) && !find_struct(tag)) return true;
    error_at(tag->str, "定義済みのタグです");
    return false;
}

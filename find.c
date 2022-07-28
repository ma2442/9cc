#include "9cc.h"

// 変数を名前で検索する。 見つからなかった場合はNULLを返す。
Var *find_var(Token *tok, Var **vars) {
    for (Var *var = *vars; var != NULL; var = var->next) {
        if (var->len == tok->len && !memcmp(var->name, tok->str, tok->len)) {
            return var;
        }
    }
    return NULL;
}
// スコープ内で定義済みの変数を検索。なければエラー
Var *fit_var(Token *tok, bool islocal) {
    Var *var = NULL;
    if (islocal) var = find_var(tok, &locals);
    if (!var) var = find_var(tok, &globals);
    if (!var) error_at(tok->str, "未定義の変数です");
    return var;
}

// 関数を名前で検索する。 見つからなかった場合はNULLを返す。
Func *find_func(Token *tok) {
    for (Func *fn = funcs; fn != NULL; fn = fn->next) {
        if (fn->len == tok->len && !memcmp(fn->name, tok->str, tok->len)) {
            return fn;
        }
    }
    return NULL;
}

// 構造体を名前で検索する。 見つからなかった場合はNULLを返す。
Struct *find_struct(Token *tok, Struct **structs) {
    for (Struct *stc = *structs; stc != NULL; stc = stc->next) {
        if (stc->len == tok->len && !memcmp(stc->name, tok->str, tok->len)) {
            return stc;
        }
    }
    return NULL;
}

// 列挙体を名前で検索する。 見つからなかった場合はNULLを返す。
Enum *find_enum(Token *tok, Enum *enums) {
    for (Enum *enm = enums; enm != NULL; enm = enm->next) {
        if (enm->len == tok->len && !memcmp(enm->name, tok->str, tok->len)) {
            return enm;
        }
    }
    return NULL;
}

// 列挙体定数を名前で検索する。 見つからなかった場合はNULLを返す。
EnumConst *find_enumconst(Token *tok, Enum *enm) {
    for (; enm != NULL; enm = enm->next) {
        for (EnumConst *cst = enm->consts; cst != NULL; cst = cst->next) {
            if (cst->len == tok->len &&
                !memcmp(cst->name, tok->str, tok->len)) {
                return cst;
            }
        }
    }
    return NULL;
}

// スコープ内で定義済みの構造体を検索。なければエラー
Struct *fit_struct(Token *tag, bool islocal) {
    Struct *stc = NULL;
    if (islocal) stc = find_struct(tag, &local_structs);
    if (!stc) stc = find_struct(tag, &global_structs);
    if (!stc) error_at(tag->str, "未定義の構造体です");
    return stc;
}

// スコープ内で定義済みの列挙体を検索。なければエラー
Enum *fit_enum(Token *tag, bool islocal) {
    Enum *enm = NULL;
    if (islocal) enm = find_enum(tag, local_enums);
    if (!enm) enm = find_enum(tag, global_enums);
    if (!enm) error_at(tag->str, "未定義の列挙体です");
    return enm;
}

// スコープ内で定義済みの列挙体定数を検索。なければNULLを返す。
EnumConst *fit_enumconst(Token *tag, bool islocal) {
    EnumConst *cst = NULL;
    if (islocal) cst = find_enumconst(tag, local_enums);
    if (!cst) cst = find_enumconst(tag, global_enums);
    if (!cst) error_at(tag->str, "未定義の定数です");
    return cst;
}

// スコープ内で定義済みのシンボルを検索。なければエラー
Symbol *fit_symbol(Token *tok, bool islocal) {
    Symbol *sym = calloc(1, sizeof(Symbol));
    if (islocal) sym->enumconst = find_enumconst(tok, local_enums);
    if (sym->enumconst) return sym;
    sym->enumconst = find_enumconst(tok, global_enums);
    if (sym->enumconst) return sym;
    if (islocal) sym->var = find_var(tok, &locals);
    if (sym->var) return sym;
    sym->var = find_var(tok, &globals);
    if (sym->var) return sym;
    sym->func = find_func(tok);
    if (sym->func) return sym;
    free(sym);
    error_at(tok->str, "未定義のシンボルです");
    return NULL;
}

// 関数、変数、定数の名前が定義可能か
bool can_def_symbol(Token *sym, bool islocal) {
    if (islocal)
        if (!find_var(sym, &locals) && !find_enumconst(sym, local_enums))
            return true;
    if (!find_var(sym, &globals) && !find_enumconst(sym, global_enums) &&
        !find_func(sym))
        return true;
    error_at(sym->str, "定義済みのシンボルです");
    return false;
}

// 構造体、列挙体のタグが定義可能か
bool can_def_tag(Token *tag, bool islocal) {
    if (islocal)
        if (!find_enum(tag, local_enums) && !find_struct(tag, &local_structs))
            return true;
    if (!find_enum(tag, global_enums) && !find_struct(tag, &global_structs))
        return true;
    error_at(tag->str, "定義済みのタグです");
    return false;
}

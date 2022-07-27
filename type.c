#include "9cc.h"

void init_sizes() {
    sizes[CHAR] = 1;
    sizes[BOOL] = 1;
    sizes[INT] = 4;
    sizes[PTR] = 8;
    sizes[STRUCT] = -1;
    sizes[ARRAY] = -1;
}

void init_words() {
    type_words[INT] = STR_INT;
    type_words[CHAR] = STR_CHAR;
    type_words[BOOL] = STR_BOOL;
    type_words[STRUCT] = STR_STRUCT;
    type_words[PTR] = "";
    type_words[ARRAY] = "";
}

// 型のサイズを計算する関数
int size(Type *typ) {
    if (typ == NULL) return -1;
    if (typ->ty == ARRAY) return size(typ->ptr_to) * typ->array_size;
    if (typ->ty == STRUCT) return typ->strct->size;
    return sizes[typ->ty];
}

// 型のアラインメントを計算する関数

// デリファレンス可能な型を判別(ARRAY, PTR)
bool can_deref(Type *typ) {
    if (typ == NULL) return false;
    if (typ->ptr_to == NULL) return false;
    return true;
}

// アラインメント(alnの倍数)に揃える
int align(int x, int aln) {
    if (aln == 0) return 1;
    // return (x + aln - 1) & ~(aln - 1);
    return ((x + aln - 1) / aln) * aln;
}

// アラインメントを計算
int calc_align(Type *type) {
    if (type->ty == ARRAY) return calc_align(type->ptr_to);
    if (type->ty == STRUCT) {
        // メンバのアラインメントの最小公倍数がアラインメント境界
        // (0,)1,2,4,8のいずれかなので単に最大値でよい
        int lcm = 0;
        for (Var *mem = type->strct->mems; mem->next != NULL; mem = mem->next) {
            int mem_align = calc_align(mem->type);
            if (lcm < mem_align) lcm = mem_align;
        }
        return lcm;
    }
    return (size(type));  // if basic type
}

// オフセットを計算･設定
int set_offset(Var *var, int base) {
    int aln = calc_align(var->type);
    // アラインする 例えばintなら4バイト境界に揃える
    var->offset = align(base, aln);
    return var->offset;
}

//ノードに型情報を付与
void typing(Node *node) {
    switch (node->kind) {
        case ND_DEREF:
            if (can_deref(node->lhs->type)) {
                node->type = node->lhs->type->ptr_to;
            }
            break;
        case ND_ADDR:
            if (node->lhs->type) {
                node->type = calloc(1, sizeof(Type));
                node->type->ty = PTR;
                node->type->ptr_to = node->lhs->type;
            }
            break;
        case ND_ADD:
        case ND_SUB:
            if (can_deref(node->lhs->type)) {
                node->type = node->lhs->type;
            } else if (can_deref(node->rhs->type)) {
                node->type = node->rhs->type;
            } else {  // 両オペランド共ポインタでない
                node->type = node->lhs->type;
            }
            break;
        case ND_MUL:
        case ND_DIV:
        case ND_MOD:
        case ND_ASSIGN:
            node->type = node->lhs->type;
            break;
        case ND_IF_ELSE:
        case ND_EQUAL:
        case ND_NOT_EQUAL:
        case ND_LESS_THAN:
        case ND_LESS_OR_EQUAL:
            node->type = calloc(1, sizeof(Type));
            node->type->ty = BOOL;
            break;
    }
}

// 型の後方にある **... を読んでポインタ型にして返す
Type *type_pointer(Type *typ) {
    while (consume("*")) {
        Type *ptr = calloc(1, sizeof(Type));
        ptr->ty = PTR;
        ptr->ptr_to = typ;
        typ = ptr;
    }
    return typ;
}

int tag_cnt = 0;

// 構造体定義
// TODO: tok_tag == NULL のパターンの定義( struct {~}..; )実装
Type *def_struct(Token *tag, Struct **stcs) {
    if (!consume("{")) return NULL;
    if (!tag) {  // タグ自動生成
        tag = calloc(1, sizeof(Token));
        tag->str = calloc(32, sizeof(char));
        sprintf(tag->str, "__struct__tag__%d__", tag_cnt++);
        tag->len = strlen(tag->str);
    }
    if (find_struct(tag, stcs)) error_at(tag->str, "定義済みの構造体です");
    Struct *stc = calloc(1, sizeof(Struct));
    stc->name = tag->str;
    stc->len = tag->len;
    // メンバに同じ構造体型を持てるように構造体リストに先行登録
    stc->next = *stcs;
    *stcs = stc;

    //メンバ初期化
    Var *mems = calloc(1, sizeof(Var));
    int ofst = 0;
    while (!consume("}")) {
        bool islocal = *stcs == local_structs;
        Type *typ = base_type(islocal);
        Token *tok = consume_ident();
        // メンバ作成（メンバのノードは不要なので放置）
        declaration_var(typ, tok, &mems);
        expect(";");
        int sz = size(mems->type);
        ofst = set_offset(mems, ofst) + sz;
    }

    (*stcs)->mems = mems;
    // 構造体のアラインメント計算、サイズをアラインメントの倍数に切り上げ
    Type *typ = calloc(1, sizeof(Type));
    typ->ty = STRUCT;
    typ->strct = *stcs;
    (*stcs)->align = calc_align(typ);
    (*stcs)->size = align(ofst, (*stcs)->align);
    return typ;
}

Type *type_struct(Struct **stcs) {
    Token *tag = consume_ident();
    // struct型を新規定義して返す
    Type *typ = def_struct(tag, stcs);
    if (typ) return typ;
    // 既存struct型を返す
    if (!tag) error_at(token->str, "構造体タグがありません");
    typ = calloc(1, sizeof(Type));
    typ->ty = STRUCT;
    bool islocal = *stcs == local_structs;
    typ->strct = fit_struct(tag, islocal);
    return typ;
}

// ( int, char,_Bool, struct (tag) ({}) ) **..
Type *base_type(bool islocal) {
    Token *tok = consume_type();
    if (!tok) return NULL;
    if (tok->kind == TK_STRUCT) {
        Struct **stcs = islocal ? &local_structs : &global_structs;
        return type_pointer(type_struct(stcs));
    }

    Type *typ = calloc(1, sizeof(Type));
    for (int tk = 0; tk < LEN_TYPE_KIND; tk++) {
        if (!strncmp(tok->str, type_words[tk], tok->len)) {
            typ->ty = tk;
            break;
        }
    }
    return type_pointer(typ);
}

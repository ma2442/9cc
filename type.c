#include "9cc.h"

void init_sizes() {
    sizes[CHAR] = 1;
    sizes[VOID] = sizes[CHAR];
    sizes[BOOL] = 1;
    sizes[SHORT] = 2;
    sizes[INT] = 4;
    sizes[LL] = 8;
    sizes[UCHAR] = 1;
    sizes[USHORT] = 2;
    sizes[UINT] = 4;
    sizes[ULL] = 8;
    sizes[ENUM] = 4;
    sizes[PTR] = 8;
    sizes[STRUCT] = -1;
    sizes[ARRAY] = -1;
}

// 型のサイズを計算する関数
int size(Type *typ) {
    if (typ == NULL) return -1;
    if (typ->ty == ARRAY) return size(typ->ptr_to) * typ->array_size;
    if (typ->ty == STRUCT) return typ->strct->stc->size;
    return sizes[typ->ty];
}

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
        for (Def *mem = type->strct->stc->mems; mem && mem->next;
             mem = mem->next) {
            int mem_align = calc_align(mem->var->type);
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

Type *new_type(TypeKind kind) {
    Type *typ = calloc(1, sizeof(Type));
    typ->ty = kind;
    return typ;
}

// 計算の型優先度 二項演算で優先度の高い方の型にあわせる
int priority(Type *typ) {
    if (typ->ty == ULL) return 4;
    if (typ->ty == LL) return 3;
    if (typ->ty == UINT) return 2;
    if (typ->ty == INT) return 1;
    if (typ->ty == USHORT || typ->ty == SHORT || typ->ty == UCHAR ||
        typ->ty == CHAR || typ->ty == BOOL)
        return 0;  // USHORT SHORT UCHAR CHAR BOOL
    return -1;     // ARRAY, PTR など
}

// 整数拡張
Type *promote_integer(Type *typ) {
    if (typ->ty == USHORT || typ->ty == SHORT || typ->ty == UCHAR ||
        typ->ty == CHAR || typ->ty == BOOL)
        return new_type(INT);
    return typ;
}

// 暗黙型
Type *implicit_type(Type *lt, Type *rt) {
    int lpy = priority(lt);
    int rpy = priority(rt);
    if (lpy == -1 || rpy == -1) return NULL; // ARRAY, PTR など処理できない型
    if (lpy == 0 && rpy == 0) return new_type(INT);  // どちらもintより小さい型
    if (lpy >= rpy) return lt;
    return rt;
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
                node->type = new_type(PTR);
                node->type->ptr_to = node->lhs->type;
            }
            break;
        case ND_ADD:
        case ND_SUB:
            if (can_deref(node->lhs->type)) {
                node->type = new_type(PTR);
                node->type->ptr_to = node->lhs->type->ptr_to;
            } else if (can_deref(node->rhs->type)) {
                node->type = new_type(PTR);
                node->type->ptr_to = node->rhs->type->ptr_to;
            } else {  // 両オペランド共ポインタでない
                node->type = implicit_type(node->lhs->type, node->rhs->type);
            }
            break;
        case ND_ASSIGN:
        case ND_ASSIGN_POST_INCDEC:
        case ND_ASSIGN_COMPOSITE:
            node->type = node->lhs->type;
            break;
        case ND_BIT_NOT:
            node->type = promote_integer(node->lhs->type);
            break;
        case ND_MUL:
        case ND_DIV:
        case ND_MOD:
        case ND_BIT_OR:
        case ND_BIT_XOR:
        case ND_BIT_AND:
        case ND_BIT_SHIFT_L:
        case ND_BIT_SHIFT_R:
            node->type = implicit_type(node->lhs->type, node->rhs->type);
            break;
        case ND_IF_ELSE:
        case ND_EQUAL:
        case ND_NOT_EQUAL:
        case ND_LESS_THAN:
        case ND_LESS_OR_EQUAL:
            node->type = new_type(BOOL);
            break;
    }
}

// 型の後方にある **... を読んでポインタ型にして返す
Type *type_pointer(Type *typ) {
    while (consume("*")) {
        Type *ptr = new_type(PTR);
        ptr->ptr_to = typ;
        typ = ptr;
    }
    return typ;
}

int tag_cnt = 0;

// 構造体か列挙体のtagを生成･チェック
Token *make_tag(Token *tag) {
    if (!tag) {  // タグ自動生成
        tag = calloc(1, sizeof(Token));
        tag->str = calloc(32, sizeof(char));
        sprintf(tag->str, "__autogen__tag__%d__", tag_cnt++);
        tag->len = strlen(tag->str);
    }
    if (!can_def_tag(tag)) return NULL;
    return tag;
}

// 構造体定義
Type *def_struct(Token *tag) {
    if (!consume("{")) return NULL;
    tag = make_tag(tag);
    if (!tag) return NULL;
    Def *stc = calloc_def(DK_STRUCT);
    stc->tok = tag;
    // メンバに同じ構造体型を持てるように構造体リストに先行登録
    Def **stcs = &def[nest]->structs;
    stc->next = *stcs;
    *stcs = stc;

    //メンバ初期化
    member_in();

    int ofst = 0;
    while (!consume("}")) {
        Token *tok_void = token;
        Type *typ = base_type();
        voidcheck(typ, tok_void->str);
        Token *tok = consume_ident();
        // メンバ作成（メンバのノードは不要なので放置）
        declaration_var(typ, tok);
        expect(";");
        int sz = size(def[nest]->vars->var->type);
        ofst = set_offset(def[nest]->vars->var, ofst) + sz;
    }

    (*stcs)->stc->mems = def[nest]->vars;
    member_out();
    // 構造体のアラインメント計算、サイズをアラインメントの倍数に切り上げ
    Type *typ = new_type(STRUCT);
    typ->strct = def[nest]->structs;
    (*stcs)->stc->align = calc_align(typ);
    (*stcs)->stc->size = align(ofst, (*stcs)->stc->align);
    return typ;
}

Type *type_struct() {
    Token *tag = consume_ident();
    // struct型を新規定義して返す
    Type *typ = def_struct(tag);
    if (typ) return typ;
    // 既存struct型を返す
    if (!tag) error_at(token->str, "構造体タグがありません");
    typ = new_type(STRUCT);
    typ->strct = fit_def(tag, DK_STRUCT);
    return typ;
}

// 列挙子定義計算用
int val(Node *node) {
    if (node->kind == ND_NUM) return node->val;
    if (node->kind == ND_IF_ELSE)
        return val(node->judge) ? val(node->lhs) : val(node->rhs);
    int l = val(node->lhs);
    int r = val(node->rhs);
    switch (node->kind) {
        case ND_EQUAL:
            return l == r;
        case ND_NOT_EQUAL:
            return l != r;
        case ND_LESS_THAN:
            return l < r;
        case ND_LESS_OR_EQUAL:
            return l <= r;
        case ND_ADD:
            return l + r;
        case ND_SUB:
            return l - r;
        case ND_MUL:
            return l * r;
        case ND_DIV:
            return l / r;
        case ND_MOD:
            return l % r;
        default:
            error("定数ではありません");
    }
}

// 列挙体定義
Type *def_enum(Token *tag) {
    if (!consume("{")) return NULL;
    tag = make_tag(tag);
    if (!tag) return NULL;
    Def *enm = calloc_def(DK_ENUM);
    enm->tok = tag;
    // 定義中列挙子も検索が可能なように先行登録
    Def **enums = &def[nest]->enums;
    enm->next = *enums;
    *enums = enm;

    // 列挙子初期化
    (*enums)->enm->consts = calloc_def(DK_ENUMCONST);
    (*enums)->enm->consts->cst->val = -1;
    do {
        Def *cst = calloc_def(DK_ENUMCONST);
        cst->cst->enm = (*enums);
        Token *tok = consume_ident();
        if (!can_def_symbol(tok)) return NULL;
        cst->tok = tok;
        if (consume("=")) {
            Node *node = condition();
            cst->cst->val = val(node);
        } else {
            cst->cst->val = (*enums)->enm->consts->cst->val + 1;
        }
        cst->next = (*enums)->enm->consts;
        (*enums)->enm->consts = cst;
    } while (consume(","));
    expect("}");

    Type *typ = new_type(ENUM);
    typ->enm = *enums;
    return typ;
}

Type *type_enum() {
    Token *tag = consume_ident();
    // enum型を新規定義して返す
    Type *typ = def_enum(tag);
    if (typ) return typ;
    // 既存enum型を返す
    if (!tag) error_at(token->str, "列挙体タグがありません");
    typ = new_type(ENUM);
    typ->enm = fit_def(tag, DK_ENUM);
    return typ;
}

// tokenと文字列の一致を調べる
bool eq(Token *tok, char *str) {
    if (!tok) return false;
    return !strncmp(tok->str, str, tok->len);
}

#define ERR_MSG_TYPEQ "型修飾子が不正です"

typedef enum {
    TYPEQ_LEN_NONE = 0,
    TYPEQ_LEN_SHORT,
    TYPEQ_LEN_LONG,
    TYPEQ_LEN_LL
} TypeqLen;

TypeqLen typeq_len() {
    Token *qlen = consume_typeq_len();
    Token *qlen2 = consume_typeq_len();
    if (!qlen) return TYPEQ_LEN_NONE;
    if (eq(qlen, STR_LONG) && eq(qlen2, STR_LONG)) return TYPEQ_LEN_LL;
    if (qlen2) error_at(qlen2->str, ERR_MSG_TYPEQ);
    if (eq(qlen, STR_LONG)) return TYPEQ_LEN_LONG;
    if (eq(qlen, STR_SHORT)) return TYPEQ_LEN_SHORT;
    error_at(qlen->str, ERR_MSG_TYPEQ);
}

TypeKind attach_qsign(TypeKind kind, Token *qsign) {
    if (eq(qsign, STR_UNSIGNED)) kind |= UNSIGNED;
    return kind;
}

// ( void, int, char, _Bool, struct or enum (tag and/or {}) ) **..
Type *base_type() {
    Token *qsign = consume_typeq_sign();
    TypeqLen qlen = typeq_len();
    Token *core = consume_type();
    Type *typ = calloc(1, sizeof(Type));
    if (!qsign && !qlen && !core) return NULL;

    // (struct|enum) (tag)? {..} | void | _Bool
    if (!qsign && !qlen) {
        if (core->kind == TK_STRUCT) return type_pointer(type_struct());
        if (core->kind == TK_ENUM) return type_pointer(type_enum());
        if (eq(core, STR_VOID)) {
            typ->ty = VOID;
            return type_pointer(typ);
        } else if (eq(core, STR_BOOL)) {
            typ->ty = BOOL;
            return type_pointer(typ);
        }
    }

    // (signed|unsigned)? char
    if (!qlen && eq(core, STR_CHAR)) {
        typ->ty = attach_qsign(CHAR, qsign);
        return type_pointer(typ);
    }

    // (signed|unsigned)? (long long|long|short)? int
    if (core && !eq(core, STR_INT)) error_at(core->str, "不正な型です");
    if (qlen == TYPEQ_LEN_LL) {
        typ->ty = attach_qsign(LL, qsign);
    } else if (qlen == TYPEQ_LEN_SHORT) {
        typ->ty = attach_qsign(SHORT, qsign);
    } else {
        typ->ty = attach_qsign(INT, qsign);
    }
    return type_pointer(typ);
}

void voidcheck(Type *typ, char *pos) {
    if (typ->ty == VOID) error_at(pos, "void型は定義できません");
}

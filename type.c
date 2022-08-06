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

bool is_signed(Type *typ) { return typ->ty & SIGNED; }

// 計算の型優先度 二項演算で優先度の高い方の型にあわせる
int priority(Type *typ) {
    if (typ->ty == ULL) return 4;
    if (typ->ty == LL) return 3;
    if (typ->ty == UINT) return 2;
    if (typ->ty == INT) return 1;
    if (typ->ty == ENUM) error("ENUMはここには来ないはず");
    if (typ->ty == STRUCT) error("STRUCTはここには来ないはず");
    if (typ->ty == USHORT || typ->ty == SHORT || typ->ty == UCHAR ||
        typ->ty == CHAR || typ->ty == BOOL)
        return 0;                   // USHORT SHORT UCHAR CHAR BOOL
    if (can_deref(typ)) return -1;  // ARRAY, PTR など
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
    if (lpy == -1 || rpy == -1) return NULL;  // ARRAY, PTR など処理できない型
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

int stcidx() { return nest - (stcnest + 1); }

// 型割当てが行われていないtypedefを探して割当て
void typing_defdtype(Token *tag, TypeKind kind, Def *dtyped) {
    for (int i = stcidx(); i > -1; i--) {
        for (Def *dtyp = def[i]->typdefs; dtyp && dtyp->next; dtyp = dtyp->next) {
            if (!sametok(tag, dtyp->tok)) continue;
            Type *typ = dtyp->type;
            while (can_deref(typ)) typ = typ->ptr_to;
            if (typ->ty != kind) continue;
            if (kind == STRUCT && !(typ->strct)) {
                typ->strct = dtyped;
                return;
            }
            if (kind == ENUM && !(typ->enm)) {
                typ->enm = dtyped;
                return;
            }
        }
    }
}

// 構造体定義
Type *def_struct(Token *tag) {
    if (!consume("{")) return NULL;
    tag = make_tag(tag);
    if (!tag) return NULL;
    Def *stc = calloc_def(DK_STRUCT);
    stc->tok = tag;
    // メンバに同じ構造体型を持てるように構造体リストに先行登録
    stc->next = def[stcidx()]->structs;
    def[stcidx()]->structs = stc;
    Def *dstc = def[stcidx()]->structs;

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

    dstc->stc->mems = def[nest]->vars;
    member_out();
    // 構造体のアラインメント計算、サイズをアラインメントの倍数に切り上げ
    Type *typ = new_type(STRUCT);
    typ->strct = dstc;
    dstc->stc->align = calc_align(typ);
    dstc->stc->size = align(ofst, dstc->stc->align);
    typing_defdtype(tag, STRUCT, dstc);
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
    Def **enums = &def[stcidx()]->enums;
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
    typing_defdtype(tag, ENUM, *enums);
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

// 型定義
void deftype(Type *typ, Token *name) {
    if (!can_def_symbol(name)) return;
    Def *dtyp = calloc_def(DK_TYPE);
    dtyp->next = def[stcidx()]->typdefs;
    dtyp->tok = name;
    dtyp->type = typ;
    def[stcidx()]->typdefs = dtyp;
}

bool typdef() {
    if (!consume("typedef")) return false;
    Token *save = token;
    Token *idt = NULL;
    Type *typ = NULL;
    Token *tag = consume_tag_without_def(TK_STRUCT);
    if (tag && !fit_def_noerr(tag, DK_STRUCT)) {  // 未定義の構造体タグ
        typ = type_pointer(new_type(STRUCT));
        idt = consume_ident();
        if (!typ || !idt) error_at(save->str, "typedefが不正です");
        deftype(type_array(typ), idt);
        expect(";");
        return true;
    }
    token = save;
    tag = consume_tag_without_def(TK_ENUM);
    if (tag && !fit_def_noerr(tag, DK_ENUM)) {  // 未定義の列挙体タグ
        typ = type_pointer(new_type(ENUM));
        idt = consume_ident();
        if (!typ || !idt) error_at(save->str, "typedefが不正です");
        deftype(type_array(typ), idt);
        expect(";");
        return true;
    }
    token = save;
    typ = base_type();
    idt = consume_ident();
    if (!typ || !idt) error_at(save->str, "typedefが不正です");
    deftype(type_array(typ), idt);
    expect(";");
    return true;
}

#define ERR_MSG_TYPEQ "型修飾子が不正です"

typedef enum {
    LENSPEC_NONE = 0,
    LENSPEC_SHORT,
    LENSPEC_LONG,
    LENSPEC_LL
} LenSpec;

LenSpec lenspec(Token *qlen, Token *qlen2) {
    if (!qlen) return LENSPEC_NONE;
    if (eqtokstr(qlen, STR_LONG) && eqtokstr(qlen2, STR_LONG)) return LENSPEC_LL;
    if (qlen2) error_at(qlen2->str, ERR_MSG_TYPEQ);
    if (eqtokstr(qlen, STR_LONG)) return LENSPEC_LONG;
    if (eqtokstr(qlen, STR_SHORT)) return LENSPEC_SHORT;
    error_at(qlen->str, ERR_MSG_TYPEQ);
}

TypeKind attach_qsign(TypeKind kind, Token *qsign) {
    if (!eqtokstr(qsign, STR_UNSIGNED)) return kind;
    return kind & ~SIGNED;
}

Type *defdtype() {
    Token *idt = consume_ident();
    if (!idt) return NULL;
    Def *typ = fit_def(idt, DK_TYPE);
    if (typ) return typ->type;
    token = idt;
    return NULL;
}

// ( void, int, char, _Bool, struct or enum (tag and/or {}) ) **..
Type *base_type() {
    Type *typ = defdtype();
    if (typ) return type_pointer(typ);
    Token *qsign = NULL;
    Token *qlen = NULL;
    Token *qlen2 = NULL;
    for (Token *q = NULL; q = consume_typeq();) {
        if (q->kind == TK_TYPEQ_SIGN && qsign ||
            q->kind == TK_TYPEQ_LENGTH && qlen && qlen2)
            error_at(q->str, ERR_MSG_TYPEQ);
        else if (q->kind == TK_TYPEQ_SIGN && !qsign)
            qsign = q;
        else if (q->kind == TK_TYPEQ_LENGTH && !qlen)
            qlen = q;
        else if (q->kind == TK_TYPEQ_LENGTH && !qlen2)
            qlen2 = q;
    }
    LenSpec lenspc = lenspec(qlen, qlen2);
    Token *core = consume_typecore();
    typ = calloc(1, sizeof(Type));
    if (!qsign && !lenspc && !core) return NULL;

    // (struct|enum) (tag)? {..} | void | _Bool
    if (!qsign && !lenspc) {
        if (core->kind == TK_STRUCT) return type_pointer(type_struct());
        if (core->kind == TK_ENUM) return type_pointer(type_enum());
        if (eqtokstr(core, STR_VOID)) {
            typ->ty = VOID;
            return type_pointer(typ);
        } else if (eqtokstr(core, STR_BOOL)) {
            typ->ty = BOOL;
            return type_pointer(typ);
        }
    }

    // (signed|unsigned)? char
    if (!lenspc && eqtokstr(core, STR_CHAR)) {
        typ->ty = attach_qsign(CHAR, qsign);
        return type_pointer(typ);
    }

    // (signed|unsigned|long|short)* int
    if (core && !eqtokstr(core, STR_INT)) error_at(core->str, "不正な型です");
    if (lenspc == LENSPEC_LL) {
        typ->ty = attach_qsign(LL, qsign);
    } else if (lenspc == LENSPEC_SHORT) {
        typ->ty = attach_qsign(SHORT, qsign);
    } else {
        typ->ty = attach_qsign(INT, qsign);
    }
    return type_pointer(typ);
}

void voidcheck(Type *typ, char *pos) {
    if (typ->ty == VOID) error_at(pos, "void型は定義できません");
}

Type *type_array(Type *typ) {
    Type head;
    Type *last = &head;
    while (consume("[")) {
        last->ptr_to = calloc(1, sizeof(Type));
        last = last->ptr_to;
        last->array_size = expect_numeric();
        last->ty = ARRAY;
        expect("]");
    }
    last->ptr_to = typ;
    return head.ptr_to;
}

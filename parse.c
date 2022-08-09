#include "9cc_manual.h"

Def *dfunc;
Def *dstrlits;
Node *statement[STMT_LEN];
Defs *def[NEST_MAX];
int breaklcnt[NEST_MAX];
int contilcnt[NEST_MAX];
Node *sw[NEST_MAX];

Def *calloc_def(DefKind kind) {
    Def *d = calloc(1, sizeof(Def));
    if (kind == DK_VAR)
        d->var = calloc(1, sizeof(Var));
    else if (kind == DK_FUNC)
        d->fn = calloc(1, sizeof(Func));
    else if (kind == DK_STRUCT)
        d->stc = calloc(1, sizeof(Struct));
    else if (kind == DK_ENUM)
        d->enm = calloc(1, sizeof(Enum));
    else if (kind == DK_ENUMCONST)
        d->cst = calloc(1, sizeof(EnumConst));
    else if (kind == DK_STRLIT)
        d->strlit = calloc(1, sizeof(StrLit));
    else if (kind == DK_TYPE)
        d->defdtype = calloc(1, sizeof(Type));
    d->kind = kind;
    return d;
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    // 型情報付与
    typing(node);
    return node;
}

Node *new_node_num(unsigned long long val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
    Token *suf = consume_numsuffix();
    if (suf) node->type->ty = suf->val;
    Type auto_typ;
    auto_typ.ty = INT;
    if (val > (1ULL << 63) - 1)
        auto_typ.ty = ULL;
    else if (val > (1LL << 32) - 1)
        auto_typ.ty = LL;
    else if (val > (1U << 31) - 1)
        auto_typ.ty = UINT;
    if (priority(&auto_typ) > priority(node->type))
        node->type->ty = auto_typ.ty;
    return node;
}

// 変数定義
Node *new_node_defvar(Type *typ, Token *var_name) {
    NodeKind kind = (nest ? ND_DEFLOCAL : ND_DEFGLOBAL);
    Node *node = new_node(kind, NULL, NULL);
    Def *dvar = calloc_def(DK_VAR);
    dvar->next = def[nest]->dvars;
    dvar->var->is_defined = true;
    dvar->tok = var_name;
    dvar->var->islocal = nest;
    dvar->var->type = typ;
    node->def = dvar;
    node->type = dvar->var->type;
    if (def[nest]->dvars) def[nest]->dvars->prev = dvar;
    def[nest]->dvars = dvar;
    return node;
}

// 変数名として識別
Node *new_node_var(Token *tok) {
    Def *dvar = fit_def(tok, DK_VAR);
    if (!dvar) return NULL;
    NodeKind kind = (dvar->var->islocal ? ND_LVAR : ND_GVAR);
    Node *node = new_node(kind, NULL, NULL);
    node->def = dvar;
    node->type = dvar->var->type;
    return node;
}

// メンバ変数として識別
Node *new_node_mem(Node *nd_stc, Token *tok) {
    member_in();
    def[nest]->dvars = nd_stc->type->dstc->stc->dmems;
    Def *dvar = find_def(tok, DK_VAR);
    member_out();
    if (!dvar) {
        error_at(tok->str, "未定義のメンバです。");
        return NULL;
    }
    Node *node = new_node(ND_MEMBER, nd_stc, NULL);
    node->def = dvar;
    node->type = dvar->var->type;
    return node;
}

Node *new_node_bool(Node *node, Node *lhs, Node *rhs) {
    Node *judge = node;
    node = new_node(ND_IF_ELSE, lhs, rhs);
    node->judge = judge;
    node->label_num = jmp_label_cnt;
    jmp_label_cnt++;
    return node;
}

// 文字列リテラル
Node *str_literal() {
    Token *tok_str = consume_str();
    if (!tok_str) return NULL;
    Type *array = calloc(1, sizeof(Type));
    array->array_size = tok_str->len - 2;  // 引用符""の分を減らす
    array->ty = ARRAY;
    array->ptr_to = calloc(1, sizeof(Type));
    array->ptr_to->ty = CHAR;

    Node *node = new_node(ND_GVAR, NULL, NULL);
    node->type = array;

    Def *dstrl = calloc_def(DK_STRLIT);
    dstrl->tok = tok_str;

    dstrl->strlit->label = calloc(DIGIT_LEN + 4, sizeof(char));
    sprintf(dstrl->strlit->label, ".LC%d", str_label_cnt);
    str_label_cnt++;
    node->def = dstrl;
    dstrl->next = dstrlits;
    dstrlits->prev = dstrl;
    dstrlits = dstrl;
    return node;
}

// 関数シグネチャ一致確認
bool eq_signature(Def *dfn1, Def *dfn2, bool check_rettype) {
    if (check_rettype && !eqtype(dfn1->fn->type, dfn2->fn->type))
        error_at(dfn2->tok->str, ERR_MSG_MISMATCH_SIGNATURE);
    Def *darg1 = dfn1->fn->darg0->prev;
    Def *darg2 = dfn2->fn->darg0->prev;
    while (darg1 || darg2) {
        if (darg1 && darg1->var->type->ty == VARIABLE) return true;
        if (darg2 && darg2->var->type->ty == VARIABLE) return true;
        if (!darg1 || !darg2 || !eqtype(darg1->var->type, darg2->var->type))
            error_at(dfn2->tok->str, ERR_MSG_MISMATCH_SIGNATURE);
        darg1 = darg1->prev;
        darg2 = darg2->prev;
    }
    return true;
}

// 関数コールノード 引数は関数名
Node *func_call(Token *tok) {
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
    Def *dfn = fit_def(tok, DK_FUNC);
    node->def = dfn;
    node->type = dfn->fn->type;

    // 実引数処理
    Def *darg = dfn->fn->darg0;
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            if (!darg->var->type || darg->var->type->ty != VARIABLE)
                darg = darg->prev;
            if (!darg) error_at(token->str, ERR_MSG_MISMATCH_SIGNATURE);
            arg->next_arg = new_node(ND_FUNC_CALL_ARG, NULL, expr());
            arg->next_arg->type = darg->var->type;
            if (!can_cast(arg->next_arg->type, arg->next_arg->rhs->type))
                error_at(token->str, ERR_MSG_MISMATCH_SIGNATURE);
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        } while (consume(","));
        expect(")");
    }
    if (darg != dfn->fn->dargs && dfn->fn->dargs->var->type->ty != VARIABLE)
        error_at(token->str, ERR_MSG_MISMATCH_SIGNATURE);
    return node;
}

// node==NULL: ++x or --x, else: x++ or x-- (++*p, p[0]++ 等もありうる)
// 前置インクリメントは複合代入と同じ処理
Node *incdec(Node *node) {
    int kind_addsub = consume_incdec();
    if (kind_addsub == -1) return NULL;
    if (node) {
        node = new_node(ND_ASSIGN_POST_INCDEC, node, NULL);
    } else {
        node = new_node(ND_ASSIGN_COMPOSITE, unary(), NULL);
    }
    Node *nd_omit = new_node(ND_OMITTED_TERM, NULL, NULL);
    nd_omit->type = node->lhs->type;
    node->rhs = new_node(kind_addsub, nd_omit, new_node_num(1));
    return node;
}

// regex = primary ( "++" | "--" | "[" expr "]" | (("."|"->")ident) )*
Node *regex() {
    Node *node = primary();
    for (;;) {
        Node *next = incdec(node);  // x++ or x--
        if (next) {
            node = next;
        } else if (consume("[")) {  // 配列添え字演算子
            node = new_node(ND_ADD, node, expr());
            node = new_node(ND_DEREF, node, NULL);
            expect("]");
        } else if (consume(".")) {
            node = new_node_mem(node, consume_ident());
        } else if (consume("->")) {
            node = new_node(ND_DEREF, node, NULL);
            node = new_node_mem(node, consume_ident());
        } else {
            break;
        }
    }
    return node;
}

// primary = strlit
//     | "(" expr ")"
//     | ident "(" ( expr (","expr)* )? ")" // func call
//     | ident
//     | num
Node *primary() {
    // 文字列リテラルとして識別
    Node *node = str_literal();
    if (node) return node;
    // 次のトークンが"("なら、"(" expr ")" のはず
    if (consume("(")) {
        node = expr();
        expect(")");
        return node;
    }
    // 変数名,関数名の識別
    Token *tok = consume_ident();
    // 識別子がなければ数値
    if (!tok) {
        Token *num = consume_numeric();
        if (!num) return NULL;
        node = new_node_num(num->val);
        return node;
    }
    // 関数名として識別
    if (consume("(")) {
        node = func_call(tok);
        if (node->type->ty == VOID)
            error_at(tok->str, "void型は評価できません");
        return node;
    }
    // 列挙子として識別
    Def *dsym = fit_def(tok, DK_ENUMCONST);
    if (dsym) return new_node_num(dsym->cst->val);
    //関数でも定数でもなければ変数
    return new_node_var(tok);
}

// 単項 unary = ("sizeof"|"++"|"--"|"+"|"-"|"*"|"&"|"!")? unary | regex
Node *unary() {
    Token *save = token;
    if (consume("(")) {  // cast
        Type *typ = base_type();
        if (!typ) {
            token = save;
            return regex();
        }
        expect(")");
        Node *node = new_node(ND_CAST, unary(), NULL);
        node->type = typ;
        return node;
    }
    if (consume("sizeof")) {
        Type *typ = base_type();
        if (!typ) typ = unary()->type;
        if (typ == NULL) {
            error("sizeof:不明な型です");
        }
        return new_node_num(size(typ));
    }
    Node *node = incdec(NULL);  // ++x or --x
    if (node) return node;
    if (consume("&")) return new_node(ND_ADDR, unary(), NULL);
    if (consume("*")) return new_node(ND_DEREF, unary(), NULL);
    if (consume("!")) return new_node(ND_EQUAL, unary(), new_node_num(0));
    if (consume("~")) return new_node(ND_BIT_NOT, unary(), NULL);
    int sign = 1;
    int cnt_sgn = 0;
    for (;; cnt_sgn++) {
        if (consume("-"))
            sign *= -1;
        else if (!consume("+"))
            break;
    }
    if (!cnt_sgn) return regex();
    Token *num = consume_numeric();
    if (num) {
        Node *node = new_node_num(num->val);
        node->val *= sign;
        return node;
    }
    if (sign == -1) return new_node(ND_SUB, new_node_num(0), unary());
    return unary();
}

Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
        } else if (consume("%")) {
            node = new_node(ND_MOD, node, unary());
        } else {
            return node;
        }
    }
}

Node *add() {
    Node *node = mul();
    for (;;) {
        if (consume("+")) {
            node = new_node(ND_ADD, node, mul());
        } else if (consume("-")) {
            node = new_node(ND_SUB, node, mul());
        } else {
            return node;
        }
    }
}

Node *bit_shift() {
    Node *node = add();
    for (;;) {
        if (consume("<<")) {
            node = new_node(ND_BIT_SHIFT_L, node, add());
        } else if (consume(">>")) {
            node = new_node(ND_BIT_SHIFT_R, node, add());
        } else {
            return node;
        }
    }
}

Node *relational() {
    Node *node = bit_shift();
    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LESS_THAN, node, bit_shift());
        } else if (consume(">")) {
            node = new_node(ND_LESS_THAN, bit_shift(), node);
        } else if (consume("<=")) {
            node = new_node(ND_LESS_OR_EQUAL, node, bit_shift());
        } else if (consume(">=")) {
            node = new_node(ND_LESS_OR_EQUAL, bit_shift(), node);
        } else {
            return node;
        }
    }
}

Node *equality() {
    Node *node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQUAL, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NOT_EQUAL, node, relational());
        } else {
            return node;
        }
    }
}

Node *bit_and() {
    Node *node = equality();
    while (consume("&")) node = new_node(ND_BIT_AND, node, equality());
    return node;
}

Node *bit_xor() {
    Node *node = bit_and();
    while (consume("^")) node = new_node(ND_BIT_XOR, node, bit_and());
    return node;
}

Node *bit_or() {
    Node *node = bit_xor();
    while (consume("|")) node = new_node(ND_BIT_OR, node, bit_xor());
    return node;
}

Node *bool_and() {
    Node *node = bit_or();
    if (consume("&&")) {
        Node *lhs = new_node(ND_NOT_EQUAL, bool_and(), new_node_num(0));
        node = new_node_bool(node, lhs, new_node_num(0));
    }
    return node;
}

Node *bool_or() {
    Node *node = bool_and();
    if (consume("||")) {
        Node *rhs = new_node(ND_NOT_EQUAL, bool_or(), new_node_num(0));
        node = new_node_bool(node, new_node_num(1), rhs);
    }
    return node;
}

// 条件演算子（三項演算子） ? :
Node *condition() {
    Node *judge = bool_or();
    if (consume("?")) {
        Node *node = new_node(ND_COND_EMPTY, NULL, NULL);
        if (!consume(":")) {
            node = expr();
            expect(":");
        }
        node = new_node(ND_IF_ELSE, node, condition());
        node->judge = judge;
        node->label_num = jmp_label_cnt;
        jmp_label_cnt++;
        return node;
    }
    return judge;
}

Node *assign() {
    Node *node = condition();
    if (consume("=")) return new_node(ND_ASSIGN, node, assign());
    // 複合代入演算子 <<= >>= |= ^= &= += -= *= /=
    int kind = consume_compo_assign();
    if (kind != -1) {
        Node *nd_assign = new_node(ND_ASSIGN_COMPOSITE, node, NULL);
        Node *nd_omit = new_node(ND_OMITTED_TERM, NULL, NULL);
        nd_omit->type = nd_assign->lhs->type;
        nd_assign->rhs = new_node(kind, nd_omit, assign());
        return nd_assign;
    }
    return node;
}

// 宣言できる変数でなければエラー
void decla_var_check(Type *typ, Token *name) {
    // 変数以外で宣言されていたらエラー
    if (find_def(name, DK_ENUMCONST) || find_def(name, DK_FUNC) ||
        find_def(name, DK_TYPE))
        error_at(name->str, "symbol has already used");
    // 同スコープにおいて同名で定義済みならエラー
    //                   違う型で宣言のみされていたらエラー
    Def *ddecla = find_def(name, DK_VAR);
    if (ddecla && ddecla->var->is_defined)
        error_at(name->str, "定義済みの変数です");
    if (ddecla && !eqtype(typ, ddecla->var->type))
        error_at(name->str, "宣言時の型と一致しません");
}

Node *decla_var(Type *typ, Token *name) {
    decla_var_check(typ, name);
    Def *dvar = calloc_def(DK_VAR);
    dvar->next = def[nest]->dvars;
    dvar->var->is_defined = false;
    dvar->tok = name;
    dvar->var->islocal = nest;
    dvar->var->type = typ;
    if (def[nest]->dvars) def[nest]->dvars->prev = dvar;
    def[nest]->dvars = dvar;
    return new_node(ND_NO_EVAL, NULL, NULL);
}

Node *defvar(Type *typ, Token *tok) {
    decla_var_check(typ, tok);
    return new_node_defvar(typ, tok);
}

Node *expr() { return assign(); }

// block {} である場合
Node *block() {
    if (!consume("{")) {
        return NULL;
    }
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->block = calloc(BLOCK_LEN, sizeof(Node *));
    for (int i = 0; i <= BLOCK_LEN; i++) {
        if (consume("}")) {
            break;
        }
        node->block[i] = localtop();
    }
    return node;
}

// decla_and_assign = declaration ("=" assign)?
Node *decla_and_assign() {
    Token *tok_void = token;
    Type *typ = base_type();
    if (!typ) return expr();
    // 変数定義
    Token *idt = consume_ident();
    if (!idt) return new_node(ND_NO_EVAL, NULL, NULL);
    voidcheck(typ, tok_void->str);
    Node *node = defvar(type_array(typ), idt);
    if (consume("="))
        node->lhs = new_node(ND_ASSIGN, new_node_var(idt), assign());
    return node;
}

Node *stmt() {
    scope_in();
    Node *node = block();
    scope_out();
    if (node) {
        // block {} の場合
        return node;
    }
    if (consume("return")) {
        if (dfunc->fn->type->ty != VOID) {
            node = expr();
            if (!node) error_at(token->str, "返却値がありません");
        }
        node = new_node(ND_RETURN, node, NULL);
        expect(";");
        return node;
    }
    if (consume("break")) {
        if (breaknest == -1)
            error_at(token->str, "breakがswitch, while, forの外側にあります");
        node = new_node(ND_GOTO, NULL, NULL);
        node->label = calloc(1, sizeof(Token));
        node->label->str = calloc(DIGIT_LEN + strlen(".Lend"), sizeof(char));
        sprintf(node->label->str, ".Lend%d", breaklcnt[breaknest]);
        node->label->len = strlen(node->label->str);
        return node;
    }
    if (consume("continue")) {
        if (continest == -1)
            error_at(token->str, "continueがwhile, forの外側にあります");
        node = new_node(ND_GOTO, NULL, NULL);
        node->label = calloc(1, sizeof(Token));
        node->label->str =
            calloc(DIGIT_LEN + strlen(".Lcontinue"), sizeof(char));
        sprintf(node->label->str, ".Lcontinue%d", contilcnt[continest]);
        node->label->len = strlen(node->label->str);
        return node;
    }
    if (consume("goto")) {
        node = new_node(ND_GOTO, NULL, NULL);
        node->label = consume_ident();
        node->fn_num = fncnt;
        return node;
    }
    if (consume("if")) {
        node = new_node(ND_IF_ELSE, NULL, NULL);
        node->label_num = jmp_label_cnt;
        jmp_label_cnt++;
        expect("(");
        node->judge = expr();
        expect(")");
        node->lhs = localtop();
        if (consume("else")) {
            node->rhs = localtop();
        }
        return node;
    }
    if (consume("switch")) {
        node = new_node(ND_SWITCH, NULL, NULL);
        switch_in();
        node->label_num = breaklcnt[breaknest];
        expect("(");
        node->judge = expr();
        expect(")");
        node->case_cnt = 0;
        node->cases = calloc(CASE_LEN, sizeof(Node *));
        swnest++;
        sw[swnest] = node;
        node->lhs = localtop();
        sw[swnest] = NULL;
        swnest--;
        switch_out();
        return node;
    }
    if (consume("do")) {
        node = new_node(ND_DO, NULL, NULL);
        loop_in();
        node->label_num = breaklcnt[breaknest];
        node->lhs = localtop();
        expect("while");
        expect("(");
        node->judge = expr();
        expect(")");
        expect(";");
        loop_out();
        return node;
    }
    if (consume("while")) {
        node = new_node(ND_WHILE, NULL, NULL);
        loop_in();
        node->label_num = breaklcnt[breaknest];
        expect("(");
        node->judge = expr();
        expect(")");
        node->lhs = localtop();
        loop_out();
        return node;
    }
    if (consume("for")) {
        node = new_node(ND_FOR, NULL, NULL);
        loop_in();
        node->label_num = breaklcnt[breaknest];
        scope_in();
        expect("(");
        if (!consume(";")) {
            node->init = decla_and_assign();
            expect(";");
        }
        if (!consume(";")) {
            node->judge = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->lhs = localtop();
        scope_out();
        loop_out();
        return node;
    }
    if (consume(";")) return new_node(ND_NO_EVAL, NULL, NULL);
    // 関数の実行のみの行(void型を許容)
    Token *save = token;
    Token *idt = consume_ident();
    if (idt && consume("(")) {
        node = func_call(idt);
        if (consume(";")) return node;
    }
    token = save;

    node = decla_and_assign();
    expect(";");
    return node;
}

// labeled = label ":" labeled | stmt
Node *labeled() {
    Node *node = NULL;
    if (swnest >= 0) {
        if (consume("case")) {
            node = new_node(ND_CASE, NULL, NULL);
            node->val = val(expr());
            node->label_num = sw[swnest]->case_cnt;
            node->sw_num = sw[swnest]->label_num;
            sw[swnest]->cases[node->label_num] = node;
            sw[swnest]->case_cnt++;
            expect(":");
            node->lhs = labeled();
            return node;
        }
        if (consume("default")) {
            node = new_node(ND_DEFAULT, NULL, NULL);
            node->sw_num = sw[swnest]->label_num;
            sw[swnest]->exists_default = true;
            expect(":");
            node->lhs = labeled();
            return node;
        }
    }
    Token *tok = consume_ident();
    if (!tok) return stmt();
    if (!consume(":")) {
        token = tok;
        return stmt();
    }
    node = new_node(ND_LABEL, labeled(), NULL);
    node->label = tok;
    node->fn_num = fncnt;
    return node;
}

Node *localtop() {
    Node *node = typdef();
    if (node) return node;
    node = labeled();
    if (node) return node;
    return new_node(ND_NO_EVAL, NULL, NULL);
}

bool decla_func(Type *typ, Token *name) {
    if (!consume("(")) return false;
    if (!name) return false;

    dfunc = calloc_def(DK_FUNC);
    dfunc->fn->type = typ;
    dfunc->tok = name;
    dfunc->fn->is_defined = false;
    scope_in();
    dfunc->fn->darg0 = def[nest]->dvars_last;

    // 仮引数処理
    if (!consume(")")) {
        do {
            if (consume("...")) {
                typ = new_type(VARIABLE);
            } else {
                Token *tok_void = token;
                typ = base_type();
                if (!typ) error_at(tok_void->str, "型ではありません");
                voidcheck(typ, tok_void->str);
                consume_ident();
            }
            Def *dvar = calloc_def(DK_VAR);
            dvar->next = def[nest]->dvars;
            dvar->var->type = typ;
            if (def[nest]->dvars) def[nest]->dvars->prev = dvar;
            def[nest]->dvars = dvar;
        } while (consume(","));
        expect(")");
    }
    // 関数情報（仮引数含む）更新
    dfunc->fn->dargs = def[nest]->dvars;
    scope_out();

    Def *ddecla = fit_def_noerr(dfunc->tok, DK_FUNC);
    if (!ddecla) {             // 既存宣言がなければ登録して終了
        can_def_symbol(name);  // シンボル使用済みエラー?
        goto REGISTER;
    }
    // 定義済関数の宣言はエラーとする
    if (ddecla->fn->is_defined) error_at(name->str, "定義済みの関数です");

    // 既存宣言とのシグネチャ一致確認
    if (!eq_signature(ddecla, dfunc, true)) return false;

REGISTER:
    dfunc->next = def[nest]->dfns;
    def[nest]->dfns = dfunc;
    return true;
}

// 関数宣言or定義ノード
Node *func(Type *typ, Token *name) {
    Token *save = token;
    // 関数宣言を試す
    if (!decla_func(typ, name)) return NULL;
    if (consume(";")) return new_node(ND_NO_EVAL, NULL, NULL);
    // 関数宣言でないなら関数定義
    token = save;

    expect("(");
    dfunc = calloc_def(DK_FUNC);
    dfunc->next = def[nest]->dfns;
    dfunc->fn->type = typ;
    dfunc->fn->is_defined = true;
    fncnt++;

    // スコープ内変数等初期化
    scope_in();
    dfunc->fn->darg0 = def[nest]->dvars_last;

    Node *node = new_node(ND_FUNC_DEFINE, NULL, NULL);
    dfunc->tok = name;
    node->def = dfunc;

    // 仮引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            if (consume("...")) break;
            Token *tok_void = token;
            typ = base_type();
            voidcheck(typ, tok_void->str);
            Token *tok = consume_ident();
            Node *ln = defvar(type_array(typ), tok);
            ln->kind = ND_LVAR;
            arg->next_arg = new_node(ND_FUNC_DEFINE_ARG, ln, NULL);
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        } while (consume(","));
        expect(")");
    }
    // 関数情報（仮引数含む）更新
    dfunc->fn->dargs = def[nest]->dvars;
    // 関数本文 "{" localtop* "}"
    node->rhs = block();
    scope_out();
    def[nest]->dfns = dfunc;
    // ローカル変数のオフセットを計算
    int ofst = 0;
    for (Def *dlcl = dfunc->fn->dvars; dlcl; dlcl = dlcl->next) {
        if (!dlcl->var->type) continue;
        int sz = size(dlcl->var->type);
        ofst = set_offset(dlcl->var, ofst + sz);
    }
    // スタックサイズを8の倍数に揃える
    dfunc->fn->stack_size = align(ofst, 8);

    return node;
}

void program() {
    int i = 0;
    scope_in();
    def[nest]->dvars = calloc_def(DK_VAR);
    dglobals_end = def[nest]->dvars;
    dstrlits = calloc_def(DK_STRLIT);
    dstrlits_end = dstrlits;
    while (!at_eof()) {
        if (typdef()) continue;
        Token *tok_void = token;
        bool is_decla = consume("extern");
        Type *typ = base_type();
        // 関数宣言･定義 または グローバル変数宣言
        Token *idt = consume_ident();
        if (!idt) {
            expect(";");
            continue;
        }
        Node *node = func(typ, idt);
        if (!node) {
            voidcheck(typ, tok_void->str);
            if (is_decla) {
                node = decla_var(type_array(typ), idt);
            } else {
                node = defvar(type_array(typ), idt);
                node->val = 0;
                if (consume("=")) node->val = val(expr());
            }
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

#include "9cc.h"

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

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    node->type = calloc(1, sizeof(Type));
    node->type->ty = INT;
    return node;
}

// 変数定義
Node *new_node_defvar(Type *typ, Token *var_name) {
    if (find_def(var_name, DK_VAR))
        error_at(var_name->str, "定義済みの変数です。");
    NodeKind kind = (nest ? ND_DEFLOCAL : ND_DEFGLOBAL);
    Node *node = new_node(kind, NULL, NULL);
    Def *var = calloc_def(DK_VAR);
    var->next = def[nest]->vars;
    var->tok = var_name;
    var->var->islocal = nest;
    var->var->type = typ;
    node->def = var;
    node->type = var->var->type;
    if (def[nest]->vars) def[nest]->vars->prev = var;
    def[nest]->vars = var;
    return node;
}

// 変数名として識別
Node *new_node_var(Token *tok) {
    Def *var = fit_def(tok, DK_VAR);
    if (!var) return NULL;
    NodeKind kind = (var->var->islocal ? ND_LVAR : ND_GVAR);
    Node *node = new_node(kind, NULL, NULL);
    node->def = var;
    node->type = var->var->type;
    return node;
}

// メンバ変数として識別
Node *new_node_mem(Node *nd_stc, Token *tok) {
    member_in();
    def[nest]->vars = nd_stc->type->strct->stc->mems;
    Def *var = find_def(tok, DK_VAR);
    member_out();
    if (!var) {
        error_at(tok->str, "未定義のメンバです。");
        return NULL;
    }
    Node *node = new_node(ND_MEMBER, nd_stc, NULL);
    node->def = var;
    node->type = var->var->type;
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

    Def *strlit = calloc_def(DK_STRLIT);
    strlit->tok = tok_str;

    strlit->strlit->label = calloc(DIGIT_LEN + 4, sizeof(char));
    sprintf(strlit->strlit->label, ".LC%d", str_label_cnt);
    str_label_cnt++;
    node->def = strlit;
    strlit->next = strlits;
    strlits->prev = strlit;
    strlits = strlit;
    return node;
}

// 関数コールノード 引数は関数名
Node *func_call(Token *tok) {
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
    Def *fn = fit_def(tok, DK_FUNC);
    if (fn) {
        node->def = fn;
        node->type = fn->fn->type;
    } else {
        node->def = calloc_def(DK_FUNC);
        node->def->tok = tok;
        // 関数が見つからなければ関数返却型をintと想定
        // (関数宣言未実装時点の処置)
        node->type = calloc(1, sizeof(Type));
        node->type->ty = INT;
    }

    // 実引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            arg->next_arg = new_node(ND_FUNC_CALL_ARG, NULL, expr());
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        } while (consume(","));
        expect(")");
    }
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
        if (num->kind == TK_CHAR) node->type->ty = CHAR;
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
    Def *sym = fit_def(tok, DK_ENUMCONST);
    if (sym) return new_node_num(sym->cst->val);
    //関数でも定数でもなければ変数
    return new_node_var(tok);
}

// 単項 unary = ("sizeof"|"++"|"--"|"+"|"-"|"*"|"&"|"!")? unary | regex
Node *unary() {
    if (consume("sizeof")) {
        Type *typ = unary()->type;
        if (typ == NULL) {
            error("sizeof:不明な型です");
        }
        return new_node_num(size(typ));
    }
    Node *node = incdec(NULL);  // ++x or --x
    if (node) return node;
    if (consume("+")) return unary();
    if (consume("-")) return new_node(ND_SUB, new_node_num(0), unary());
    if (consume("&")) return new_node(ND_ADDR, unary(), NULL);
    if (consume("*")) return new_node(ND_DEREF, unary(), NULL);
    if (consume("!"))
        return new_node_bool(unary(), new_node_num(0), new_node_num(1));
    return regex();
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

Node *relational() {
    Node *node = add();
    for (;;) {
        if (consume("<")) {
            node = new_node(ND_LESS_THAN, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LESS_THAN, add(), node);
        } else if (consume("<=")) {
            node = new_node(ND_LESS_OR_EQUAL, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_LESS_OR_EQUAL, add(), node);
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

Node *bool_and() {
    Node *node = equality();
    if (consume("&&")) node = new_node_bool(node, bool_and(), new_node_num(1));
    return node;
}

Node *bool_or() {
    Node *node = bool_and();
    if (consume("||")) node = new_node_bool(node, new_node_num(1), bool_or());
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
    // 複合代入演算子 += -= *= /=
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

Node *declaration_var(Type *typ, Token *tok) {
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
    return new_node_defvar(head.ptr_to, tok);
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
        node->block[i] = labeled();
    }
    return node;
}

// decla_and_assign = declaration ("=" assign)?
Node *decla_and_assign() {
    Type *typ = base_type();
    if (!typ) return expr();
    // 変数定義
    Token *idt = consume_ident();
    if (!idt) return new_node(ND_NO_EVAL, NULL, NULL);
    voidcheck(typ, tok_type->str);
    Node *node = declaration_var(typ, idt);
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
        if (fnc->fn->type->ty != VOID) {
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
        node->lhs = labeled();
        if (consume("else")) {
            node->rhs = labeled();
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
        node->lhs = labeled();
        sw[swnest] = NULL;
        swnest--;
        switch_out();
        return node;
    }
    if (consume("do")) {
        node = new_node(ND_DO, NULL, NULL);
        loop_in();
        node->label_num = breaklcnt[breaknest];
        node->lhs = labeled();
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
        node->lhs = labeled();
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
        node->lhs = labeled();
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

// 関数定義ノード
Node *func(Type *typ, Token *func_name) {
    if (!consume("(")) return NULL;
    if (!func_name) return NULL;
    fnc = calloc_def(DK_FUNC);
    fnc->next = def[nest]->funcs;
    fnc->fn->type = typ;
    fncnt++;

    // スコープ内変数等初期化
    scope_in();

    Node *node = new_node(ND_FUNC_DEFINE, NULL, NULL);
    fnc->tok = func_name;
    node->def = fnc;

    // 仮引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            typ = base_type();
            voidcheck(typ, tok_type->str);
            Token *tok = consume_ident();
            Node *ln = declaration_var(typ, tok);
            ln->kind = ND_LVAR;
            arg->next_arg = new_node(ND_FUNC_DEFINE_ARG, ln, NULL);
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        } while (consume(","));
        expect(")");
    }
    // 関数情報（仮引数含む）更新
    fnc->fn->args = def[nest]->vars;
    // 関数本文 "{" labeled* "}"
    node->rhs = block();
    scope_out();
    def[nest]->funcs = fnc;
    // ローカル変数のオフセットを計算
    int ofst = 0;
    for (Def *lcl = fnc->fn->vars; lcl; lcl = lcl->next) {
        if (!lcl->var->type) continue;
        int sz = size(lcl->var->type);
        ofst = set_offset(lcl->var, ofst + sz);
    }
    // スタックサイズを8の倍数に揃える
    fnc->fn->stack_size = align(ofst, 8);

    return node;
}

void program() {
    int i = 0;
    scope_in();
    def[nest]->vars = calloc_def(DK_VAR);
    globals_end = def[nest]->vars;
    strlits = calloc_def(DK_STRLIT);
    strlits_end = strlits;
    while (!at_eof()) {
        Type *typ = base_type();
        // 関数定義 または グローバル変数宣言
        Token *idt = consume_ident();
        if (!idt) {
            expect(";");
            continue;
        }
        Node *node = func(typ, idt);
        if (!node) {
            voidcheck(typ, tok_type->str);
            node = declaration_var(typ, idt);
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

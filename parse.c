#include "9cc.h"
enum { GLOBAL, LOCAL } IsLocal;

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

// ラベル通し番号
int jmp_label_cnt = 0;
int str_label_cnt = 0;

// break対象ラベル番号とbreakネスト数
int breaklcnt[NEST_MAX];
int breaknest = -1;
// continue対象ラベル番号とネスト数
int contilcnt[NEST_MAX];
int continest = -1;

// switch対象ノード
Node *sw[NEST_MAX];
int swcnt = -1;

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at(char *loc, char *fmt, ...) {
    // locが含まれている行の開始地点と終了地点を取得
    char *line = loc;
    while (user_input < line && line[-1] != '\n') line--;

    char *end = loc;
    while (*end != '\n') end++;

    // 見つかった行が全体の何行目なのかを調べる
    int line_num = 1;
    for (char *p = user_input; p < line; p++)
        if (*p == '\n') line_num++;

    // 見つかった行を、ファイル名と行番号と一緒に表示
    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // エラー箇所を"^"で指し示して、エラーメッセージを表示
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
    fprintf(stderr, "^ ");
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fprintf(stderr, "\n");
    exit(1);
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
    switch (token->kind) {
        case TK_RESERVED:
        case TK_SIZEOF:
        case TK_RETURN:
        case TK_CTRL:
        case TK_STRUCT:
            break;
        default:
            return false;
    }
    if (strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

Token *consume_if_kind_is(TokenKind tk) {
    if (token->kind != tk) {
        return NULL;
    }
    Token *this = token;
    token = token->next;
    return this;
}
Token *consume_numeric() {
    Token *tok = consume_if_kind_is(TK_CHAR);
    if (tok) return tok;
    return consume_if_kind_is(TK_NUM);
}
Token *consume_str() { return consume_if_kind_is(TK_STR); }
Token *consume_type() {
    Token *tok = consume_if_kind_is(TK_STRUCT);
    if (tok) return tok;
    tok = consume_if_kind_is(TK_TYPE);
    if (tok) return tok;
    return consume_if_kind_is(TK_ENUM);
}
Token *consume_ident() { return consume_if_kind_is(TK_IDENT); }

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if ((token->kind != TK_RESERVED && token->kind != TK_CTRL) ||
        strlen(op) != token->len || memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
    return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_numeric() {
    if (token->kind != TK_NUM && token->kind != TK_CHAR) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

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
    NodeKind kind = (var->var->islocal ? ND_LVAR : ND_GVAR);
    Node *node = new_node(kind, NULL, NULL);
    node->def = var;
    node->type = var->var->type;
    return node;
}

// スコープに入る
void scope_in() {
    nest++;
    def[nest] = calloc(1, sizeof(Defs));
    def[nest]->funcs = calloc_def(DK_FUNC);
    def[nest]->vars = calloc_def(DK_VAR);
    def[nest]->vars_last = def[nest]->vars;
    def[nest]->structs = calloc_def(DK_STRUCT);
}

// スコープから出る
void scope_out() {
    if (nest > 0 && def[nest]->vars_last) {
        // 関数内ネストの変数はローカル変数に載せて行く
        def[nest]->vars_last->next = fnc->fn->vars;
        fnc->fn->vars = def[nest]->vars;
    }
    free(def[nest]);
    def[nest] = NULL;
    nest--;
}

// 構造体メンバのスコープに入る
void member_in() { scope_in(); }

// 構造体メンバのスコープから出る
void member_out() {
    free(def[nest]);
    def[nest] = NULL;
    nest--;
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

// for, while, do-while スコープに入る際のbreak, continueラベル同期処理
void loop_in() {
    breaknest++;
    continest++;
    breaklcnt[breaknest] = jmp_label_cnt;
    contilcnt[continest] = jmp_label_cnt;
    jmp_label_cnt++;
}

// for, while, do-while スコープから出る際のbreak, continueラベル同期処理
void loop_out() {
    breaknest--;
    continest--;
}

// switch スコープに入る際ののbreakラベル同期処理
void switch_in() {
    breaknest++;
    breaklcnt[breaknest] = jmp_label_cnt;
    jmp_label_cnt++;
}

// switch スコープから出る際のbreakラベル同期処理
void switch_out() { breaknest--; }

Node *code[CODE_LEN];
Node *statement[STMT_LEN];

Node *expr();

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

int consume_incdec() {
    if (consume("++")) return ND_ADD;
    if (consume("--")) return ND_SUB;
    return -1;
}

// node==NULL: ++x or --x, else: x++ or x-- (++*p, p[0]++ 等もありうる)
// 前置インクリメントは複合代入と同じ処理
Node *incdec(Node *node) {
    int kind = consume_incdec();
    if (kind == -1) return NULL;
    int asn_kind = ASN_COMPOSITE;
    if (!node) {
        node = unary();
    } else {
        asn_kind = ASN_POST_INCDEC;
    }
    node = new_node(ND_ASSIGN, node, NULL);
    node->assign_kind = asn_kind;
    Node *nd_typ = new_node(ND_DUMMY, NULL, NULL);
    nd_typ->type = node->lhs->type;
    node->rhs = new_node(kind, nd_typ, new_node_num(1));
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
        node = new_node_num(num->val);
        if (num->kind == TK_CHAR) node->type->ty = CHAR;
        return node;
    }
    // 関数名として識別
    if (consume("(")) return func_call(tok);
    // 列挙子として識別
    Def *sym = fit_def(tok, DK_ENUMCONST);
    if (sym) return new_node_num(sym->cst->val);
    //関数でも定数でもなければ変数
    return new_node_var(tok);
}

//複合代入
int consume_compo_assign() {
    if (consume("+=")) return ND_ADD;
    if (consume("-=")) return ND_SUB;
    if (consume("*=")) return ND_MUL;
    if (consume("/=")) return ND_DIV;
    if (consume("%=")) return ND_MOD;
    return -1;
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
        Node *nd_assign = new_node(ND_ASSIGN, node, NULL);
        nd_assign->assign_kind = ASN_COMPOSITE;
        Node *nd_typ = new_node(ND_DUMMY, NULL, NULL);
        nd_typ->type = nd_assign->lhs->type;
        nd_assign->rhs = new_node(kind, nd_typ, assign());
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
        node = new_node(ND_RETURN, expr(), NULL);
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
        swcnt++;
        sw[swcnt] = node;
        node->lhs = labeled();
        sw[swcnt] = NULL;
        swcnt--;
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
    node = decla_and_assign();
    expect(";");
    return node;
}

// labeled = label ":" labeled | stmt
Node *labeled() {
    Node *node = NULL;
    if (swcnt >= 0) {
        if (consume("case")) {
            node = new_node(ND_CASE, NULL, NULL);
            node->val = val(expr());
            node->case_num = sw[swcnt]->case_cnt;
            node->sw_num = sw[swcnt]->label_num;
            sw[swcnt]->cases[node->label_num] = node;
            sw[swcnt]->case_cnt++;
            expect(":");
            node->lhs = labeled();
            return node;
        }
        if (consume("default")) {
            node = new_node(ND_DEFAULT, NULL, NULL);
            node->sw_num = sw[swcnt]->label_num;
            sw[swcnt]->exists_default = true;
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
    return node;
}

// 関数定義ノード
Node *func(Type *typ, Token *func_name) {
    if (!consume("(")) return NULL;
    if (!func_name) return NULL;
    fnc = calloc_def(DK_FUNC);
    fnc->next = def[nest]->funcs;
    fnc->fn->type = typ;

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
    nest = -1;
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
            node = declaration_var(typ, idt);
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

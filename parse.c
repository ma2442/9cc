#include "9cc.h"
enum { GLOBAL, LOCAL } IsLocal;
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
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%s'ではありません", op);
    }
    token = token->next;
    return;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
    if (token->kind != TK_NUM) {
        error_at(token->str, "数ではありません");
    }
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// 変数定義
Node *new_node_defvar(Type *typ, Token *var_name, Var **vars) {
    NodeKind kind = (*vars == locals ? ND_DEFLOCAL : ND_DEFGLOBAL);
    Node *node = new_node(kind, NULL, NULL);
    Var *var = find_var(var_name, *vars);
    if (var) error_at(var_name->str, "定義済みの変数です。");

    var = calloc(1, sizeof(Var));
    var->next = *vars;
    (*vars)->prev = var;
    var->name = var_name->str;
    var->len = var_name->len;
    var->type = typ;
    node->var = var;
    node->type = var->type;
    *vars = var;
    return node;
}

// 変数名として識別
Node *new_node_var(Token *tok) {
    // ローカル変数チェック
    Var *var = find_var(tok, locals);
    NodeKind kind = (var ? ND_LVAR : ND_GVAR);
    // グローバル変数チェック
    if (!var) var = find_var(tok, globals);
    if (!var) {
        error_at(tok->str, "未定義の変数です。");
        return NULL;
    }
    Node *node = new_node(kind, NULL, NULL);
    node->var = var;
    node->type = var->type;
    return node;
}

// メンバ変数として識別
Node *new_node_mem(Node *nd_stc, Token *tok) {
    // ローカル変数チェック
    Var *var = find_var(tok, nd_stc->type->strct->mems);
    if (!var) {
        error_at(tok->str, "未定義のメンバです。");
        return NULL;
    }
    Node *node = new_node(ND_MEMBER, nd_stc, NULL);
    node->var = var;
    node->type = var->type;
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

    StrLit *strlit = calloc(1, sizeof(StrLit));
    strlit->str = tok_str->str;
    strlit->len = tok_str->len;

    strlit->name = calloc(str_label_cnt + 4, sizeof(char));
    sprintf(strlit->name, ".LC%d", str_label_cnt);
    str_label_cnt++;
    node->strlit = strlit;
    strlit->next = strlits;
    strlits->prev = strlit;
    strlits = strlit;
    return node;
}

// 関数コールノード 引数は関数名
Node *func_call(Token *tok) {
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
    Func *fn = find_func(tok);
    if (fn) {
        node->func = fn;
        node->type = fn->type;
    } else {
        node->func = calloc(1, sizeof(Func));
        node->func->name = tok->str;
        node->func->len = tok->len;
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
    if (!tok) return new_node_num(expect_number());
    // 関数名として識別
    if (consume("(")) return func_call(tok);
    // 列挙体定数として識別
    Symbol *sym = fit_symbol(tok, locals ? true : false);
    if (sym->enumconst) return new_node_num(sym->enumconst->val);
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

Node *assign() {
    Node *node = bool_or();
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

Node *declaration_var(Type *typ, Token *tok, Var **vars) {
    Type head;
    Type *last = &head;
    while (consume("[")) {
        last->ptr_to = calloc(1, sizeof(Type));
        last = last->ptr_to;
        last->array_size = expect_number();
        last->ty = ARRAY;
        expect("]");
    }
    last->ptr_to = typ;
    return new_node_defvar(head.ptr_to, tok, vars);
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
        node->block[i] = stmt();
    }
    return node;
}

Node *stmt() {
    Node *node = block();
    if (node) {
        // block {} の場合
        return node;
    }
    if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
        return node;
    }
    if (consume("if")) {
        expect("(");
        Node *judge = expr();
        expect(")");
        node = new_node(ND_IF_ELSE, stmt(), NULL);
        node->judge = judge;
        if (consume("else")) {
            node->rhs = stmt();
        }
        node->label_num = jmp_label_cnt;
        jmp_label_cnt++;
        return node;
    }
    if (consume("while")) {
        expect("(");
        Node *judge = expr();
        expect(")");
        node = new_node(ND_WHILE, stmt(), NULL);
        node->judge = judge;
        node->label_num = jmp_label_cnt;
        jmp_label_cnt++;
        return node;
    }
    if (consume("for")) {
        expect("(");
        Node *init = NULL;
        if (!consume(";")) {
            init = expr();
            expect(";");
        }
        Node *judge = NULL;
        if (!consume(";")) {
            judge = expr();
            expect(";");
        }
        Node *inc = NULL;
        if (!consume(")")) {
            inc = expr();
            expect(")");
        }
        node = new_node(ND_FOR, stmt(), NULL);
        node->init = init;
        node->judge = judge;
        node->inc = inc;
        node->label_num = jmp_label_cnt;
        jmp_label_cnt++;
        return node;
    }
    Type *typ = base_type(LOCAL);
    if (typ) {  // 変数定義
        Token *idt = consume_ident();
        if (!idt) {
            expect(";");
            return new_node(ND_NO_EVAL, NULL, NULL);
        }
        node = declaration_var(typ, idt, &locals);
        if (consume("="))
            node->lhs = new_node(ND_ASSIGN, new_node_var(idt), assign());
        expect(";");
        return node;
    }
    node = expr();
    expect(";");
    return node;
}

// 関数定義ノード
Node *func(Type *typ, Token *func_name) {
    if (!consume("(")) return NULL;
    if (!func_name) return NULL;
    Func *fn = calloc(1, sizeof(Func));
    fn->next = funcs;
    fn->type = typ;

    //ローカルの構造体定義初期化
    local_structs = calloc(1, sizeof(Struct));
    //ローカル変数初期化
    locals = calloc(1, sizeof(Var));

    Node *node = new_node(ND_FUNC_DEFINE, NULL, NULL);
    fn->name = func_name->str;
    fn->len = func_name->len;
    node->func = fn;

    // 仮引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            typ = base_type(LOCAL);
            Token *tok = consume_ident();
            Node *ln = declaration_var(typ, tok, &locals);
            ln->kind = ND_LVAR;
            arg->next_arg = new_node(ND_FUNC_DEFINE_ARG, ln, NULL);
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        } while (consume(","));
        expect(")");
    }
    // 関数情報（仮引数含む）更新
    fn->args = locals;
    funcs = fn;
    // 関数本文 "{" stmt* "}"
    node->rhs = block();
    // ローカル変数のオフセットを計算
    int ofst = 0;
    for (Var *lcl = locals; lcl->next != NULL; lcl = lcl->next) {
        int sz = size(lcl->type);
        ofst = set_offset(lcl, ofst + sz);
    }
    // スタックサイズを8の倍数に揃える
    fn->stack_size = align(ofst, 8);

    locals = NULL;
    local_structs = NULL;
    local_enums = NULL;
    return node;
}

void program() {
    int i = 0;
    funcs = calloc(1, sizeof(Func));
    strlits = calloc(1, sizeof(StrLit));
    strlits_end = strlits;
    globals = calloc(1, sizeof(Var));
    globals_end = globals;
    global_structs = calloc(1, sizeof(Struct));
    while (!at_eof()) {
        Type *typ = base_type(GLOBAL);
        // 関数定義 または グローバル変数宣言
        Token *idt = consume_ident();
        if (!idt) {
            expect(";");
            continue;
        }
        Node *node = func(typ, idt);
        if (!node) {
            node = declaration_var(typ, idt, &globals);
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

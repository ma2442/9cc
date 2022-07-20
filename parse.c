#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    // 型情報付与
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
        case ND_ASSIGN:
            node->type = node->lhs->type;
            break;
        case ND_EQUAL:
        case ND_NOT_EQUAL:
        case ND_LESS_THAN:
        case ND_LESS_OR_EQUAL:
            node->type = calloc(1, sizeof(Type));
            node->type->ty = INT;
            break;
    }
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
Token *consume_type() { return consume_if_kind_is(TK_TYPE); }
Token *consume_ident() { return consume_if_kind_is(TK_IDENT); }

// 変数を名前で検索する。 見つからなかった場合はNULLを返す。
Var *find_var(Token *tok, Var **vars) {
    for (Var *var = *vars; var != NULL; var = var->next) {
        if (var->len == tok->len && !memcmp(var->name, tok->str, tok->len)) {
            return var;
        }
    }
    return NULL;
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
Node *new_node_defvar(Token *tok, Type *typ, Var **vars) {
    Node *node;
    if (*vars == locals) {
        node = new_node(ND_DEFLOCAL, NULL, NULL);
    } else {
        node = new_node(ND_DEFGLOBAL, NULL, NULL);
    }
    Var *lvar = find_var(tok, vars);

    if (lvar) {
        // エラー 定義済み
        error_at(tok->str, "定義済みの変数です。");
    }
    lvar = calloc(1, sizeof(Var));
    lvar->next = *vars;
    (*vars)->prev = lvar;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->type = typ;
    // ローカル変数はオフセット設定
    if (*vars == locals) {
        lvar->offset = (locals ? locals->offset : 0);
        lvar->offset += size(typ);
        lvar->offset = ((lvar->offset + 7) / 8) * 8;  // 8の倍数に揃える
        node->offset = lvar->offset;
    } else {  //グローバル変数は名前設定
        node->name = lvar->name;
        node->name_len = lvar->len;
    }
    node->type = lvar->type;
    *vars = lvar;
    return node;
}

// 変数名として識別
Node *new_node_var(Token *tok) {
    // ローカル変数チェック
    Var *lvar = find_var(tok, &locals);
    if (lvar) {
        Node *node = new_node(ND_LVAR, NULL, NULL);
        node->offset = lvar->offset;
        node->type = lvar->type;
        return node;
    }
    // グローバル変数チェック
    lvar = find_var(tok, &globals);
    if (!lvar) {
        error_at(tok->str, "未定義の変数です。");
        return NULL;
    }
    Node *node = new_node(ND_GVAR, NULL, NULL);
    node->name = tok->str;
    node->name_len = tok->len;
    node->type = lvar->type;
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
    sprintf(strlit->name, ".LC%d", str_label_cnt);
    str_label_cnt++;
    node->name = strlit->name;
    node->name_len = strlen(strlit->name);
    strlit->next = strlits;
    strlits->prev = strlit;
    strlits = strlit;
    return node;
}

// 関数コールノード 引数は関数名
Node *func_call(Token *tok) {
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
    node->name = tok->str;
    node->name_len = tok->len;
    Func *fn = find_func(tok);
    if (fn) {
        node->type = fn->type;
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
    //関数でなければ変数
    if (!consume("(")) return new_node_var(tok);
    // 関数名として識別
    return func_call(tok);
}

int consume_incdec() {
    if (consume("++")) return ND_ADD;
    if (consume("--")) return ND_SUB;
    return -1;
}

// tok==NULL: ++x or --x, else: x++ or x-- (++*p, p[0]++ 等もありうる)
Node *incdec(bool is_post) {
    Node *node = NULL;
    if (is_post) node = unary();
    int kind = consume_incdec();
    if (kind == -1) return node;
    if (!is_post) node = unary();
    Node *nd_assign = new_node(ND_ASSIGN, node, NULL);
    nd_assign->assign_kind = (is_post ? ASN_POST_INCDEC : ASN_PRE_INCDEC);
    Node *nd_typ = new_node(ND_DUMMY, NULL, NULL);
    nd_typ->type = nd_assign->lhs->type;
    nd_assign->rhs = new_node(kind, nd_typ, new_node_num(1));
    return nd_assign;
}
Node *post_incdec() { return incdec(true); }
Node *pre_incdec() { return incdec(false); }

// 単項
Node *unary() {
    if (consume("sizeof")) {
        Type *typ = post_incdec()->type;
        if (typ == NULL) {
            error("sizeof:不明な型です");
        }
        return new_node_num(size(typ));
    }
    Node *node = pre_incdec();
    if (node) return node;
    if (consume("+")) return post_incdec();
    if (consume("-")) return new_node(ND_SUB, new_node_num(0), post_incdec());
    if (consume("&")) return new_node(ND_ADDR, post_incdec(), NULL);
    if (consume("*")) return new_node(ND_DEREF, post_incdec(), NULL);

    node = primary();
    while (consume("[")) {  // 配列添え字演算子
        node = new_node(ND_ADD, node, expr());
        node = new_node(ND_DEREF, node, NULL);
        expect("]");
    }
    return node;
}

Node *mul() {
    Node *node = post_incdec();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, post_incdec());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, post_incdec());
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

Node *assign() {
    Node *node = equality();
    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

Type *type(Token *tok) {
    Type *typ = calloc(1, sizeof(Type));
    char *words[LEN_TYPE_KIND];
    words[INT] = "int";
    words[CHAR] = "char";
    for (int tk = 0; tk < LEN_TYPE_KIND; tk++) {
        if (!strncmp(tok->str, words[tk], tok->len)) {
            typ->ty = tk;
            break;
        }
    }
    while (consume("*")) {
        Type *ptr = calloc(1, sizeof(Type));
        ptr->ty = PTR;
        ptr->ptr_to = typ;
        typ = ptr;
    }
    return typ;
}

Node *declaration_after_ident(Type *typ, Token *tok, Var **vars) {
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
    return new_node_defvar(tok, head.ptr_to, vars);
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
    Token *tok = consume_type();
    if (tok) {
        Type *typ = type(tok);
        tok = consume_ident();
        node = declaration_after_ident(typ, tok, &locals);
        if (consume("=")) {
            node->lhs = new_node(ND_ASSIGN, new_node_var(tok), assign());
        }
        expect(";");
        return node;
    }
    node = expr();
    expect(";");
    return node;
}

// 関数定義ノード
Node *func_after_leftparen(Type *typ, Token *func_name) {
    if (!func_name) {
        return NULL;  // エラー
    }
    Func *fn = calloc(1, sizeof(Func));
    fn->next = funcs;
    fn->type = typ;

    //ローカル変数初期化
    locals = calloc(1, sizeof(Var));

    Node *node = new_node(ND_FUNC_DEFINE, NULL, NULL);
    fn->name = func_name->str;
    fn->len = func_name->len;
    node->name = func_name->str;
    node->name_len = func_name->len;

    // 仮引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            Token *tok = consume_type();
            typ = type(tok);
            tok = consume_ident();
            Node *ln = declaration_after_ident(typ, tok, &locals);
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
    // 変数分の確保領域を記憶
    node->offset = locals->offset;
    return node;
}

void program() {
    int i = 0;
    funcs = calloc(1, sizeof(Func));
    strlits = calloc(1, sizeof(StrLit));
    strlits_end = strlits;
    globals = calloc(1, sizeof(Var));
    globals_end = globals;
    while (!at_eof()) {
        Token *tok = consume_type();
        Type *typ = type(tok);
        tok = consume_ident();
        Node *node;
        if (consume("(")) {
            node = func_after_leftparen(typ, tok);
        } else {
            node = declaration_after_ident(typ, tok, &globals);
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

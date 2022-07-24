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
    return consume_if_kind_is(TK_TYPE);
}
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

// 構造体を名前で検索する。 見つからなかった場合はNULLを返す。
Struct *find_struct(Token *tok) {
    for (Struct *stc = structs; stc != NULL; stc = stc->next) {
        if (stc->len == tok->len && !memcmp(stc->name, tok->str, tok->len)) {
            return stc;
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
    Var *var = find_var(tok, vars);

    if (var) {
        // エラー 定義済み
        error_at(tok->str, "定義済みの変数です。");
    }
    var = calloc(1, sizeof(Var));
    var->next = *vars;
    (*vars)->prev = var;
    var->name = tok->str;
    var->len = tok->len;
    var->type = typ;
    node->var = var;
    node->type = var->type;
    *vars = var;
    return node;
}

// 変数名として識別
Node *new_node_var(Token *tok) {
    // ローカル変数チェック
    Var *var = find_var(tok, &locals);
    NodeKind kind = (var ? ND_LVAR : ND_GVAR);
    // グローバル変数チェック
    if (!var) var = find_var(tok, &globals);
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
    Var *var = find_var(tok, &(nd_stc->type->strct->mems));
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
    //関数でなければ変数
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

Type *type(Token *tok) {
    Type *typ = calloc(1, sizeof(Type));
    for (int tk = 0; tk < LEN_TYPE_KIND; tk++) {
        if (!strncmp(tok->str, type_words[tk], tok->len)) {
            typ->ty = tk;
            break;
        }
    }
    if (typ->ty == STRUCT) {
        tok = consume_ident();
        typ->strct = find_struct(tok);
        // if (!typ->strct) return NULL;
        if (!typ->strct) error_at(tok->str, "未定義の構造体です");
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

// アラインメント(alnの倍数)に揃える
int align(int x, int aln) {
    // return (x + aln - 1) & ~(aln - 1);
    return ((x + aln - 1) / aln) * aln;
}

// オフセットを計算･設定
int set_offset(Var *var, int base) {
    Type *prm = var->type;
    while (prm->ty == ARRAY) prm = prm->ptr_to;
    int aln = (prm->ty == STRUCT ? 16 : size(prm));
    // オフセット境界 例えばintなら4バイト境界に揃える
    var->offset = align(base, aln);
    return var->offset;
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
    node->func = fn;

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
    // ローカル変数のオフセットを計算
    int ofst = 0;
    for (Var *lcl = locals; lcl->next != NULL; lcl = lcl->next) {
        int sz = size(lcl->type);
        ofst = set_offset(lcl, ofst + sz);
    }
    // スタックサイズを8の倍数に揃える
    fn->stack_size = align(ofst, 8);
    return node;
}

// 構造体定義ノード
void struct_after_leftbrace(Token *tok) {
    if (find_struct(tok)) error_at(tok->str, "構造体の定義が重複しています");
    Struct *stc = calloc(1, sizeof(Struct));
    stc->name = tok->str;
    stc->len = tok->len;
    // メンバに同じ構造体型を持てるように構造体リストに先行登録
    stc->next = structs;
    structs = stc;

    //メンバ初期化
    locals = calloc(1, sizeof(Var));
    int ofst = 0;
    while (!consume("}")) {
        Type *typ = type(consume_type());
        tok = consume_ident();
        // メンバ作成（メンバのノードは不要なので放置）
        declaration_after_ident(typ, tok, &locals);
        expect(";");
        int sz = size(locals->type);
        ofst = set_offset(locals, ofst) + sz;
    }
    expect(";");
    structs->mems = locals;
    // 構造体サイズを8の倍数に揃える
    structs->align = 8;
    structs->size = align(ofst, structs->align);
}

void program() {
    int i = 0;
    funcs = calloc(1, sizeof(Func));
    strlits = calloc(1, sizeof(StrLit));
    strlits_end = strlits;
    globals = calloc(1, sizeof(Var));
    globals_end = globals;
    structs = calloc(1, sizeof(Struct));
    while (!at_eof()) {
        Token *tok_typ = consume_type();
        Token *tok_idt = consume_ident();
        // 構造体定義
        // TODO: tok_idt == NULL のパターンの定義( struct {~}..; )実装
        if (tok_typ->kind == TK_STRUCT && tok_idt && consume("{")) {
            struct_after_leftbrace(tok_idt);
            continue;
        }
        // 関数定義 または グローバル変数宣言
        token = tok_typ->next;
        Type *typ = type(tok_typ);
        tok_idt = consume_ident();
        Node *node;
        if (consume("(")) {
            node = func_after_leftparen(typ, tok_idt);
        } else {
            node = declaration_after_ident(typ, tok_idt, &globals);
            expect(";");
        }
        code[i] = node;
        i++;
    }
    code[i] = NULL;
}

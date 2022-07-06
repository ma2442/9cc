#include "9cc.h"

// ポインタが参照する型のサイズを返す
int size_deref(Node *node) {
    if (node->type == NULL || node->type->ty != PTR) {
        return -1;
    }
    return sizes[node->type->ptr_to->ty];
}

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    // 型情報付与
    if (node->kind == ND_DEREF) {
        if (node->lhs->type && node->lhs->type->ptr_to) {
            node->type = node->lhs->type->ptr_to;
        }
    } else if (node->kind == ND_ADDR) {
        if (node->lhs->type) {
            node->type = calloc(1, sizeof(Type));
            node->type->ty = PTR;
            node->type->ptr_to = node->lhs->type;
        }
    } else if (node->kind == ND_ADD || node->kind == ND_SUB) {
        if (size_deref(node->lhs) != -1) {
            node->type = node->lhs->type;
        } else if (size_deref(node->rhs) != -1) {
            node->type = node->rhs->type;
        }
    }
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

// 現在着目しているトークン
Token *token;

// 入力プログラム
char *user_input;

// ラベル通し番号
int label_cnt = 0;

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " ");  // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
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

Token *consume_ident() {
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *this = token;
    token = token->next;
    return this;
}

// 変数を名前で検索する。 見つからなかった場合はNULLを返す。
LVar *find_lvar(Token *tok) {
    for (LVar *var = locals; var != NULL; var = var->next) {
        if (var->len == tok->len && !memcmp(var->name, tok->str, tok->len)) {
            return var;
        }
    }
    return NULL;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "'%c'ではありません", op);
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

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

// 変数定義
Node *new_node_deflocal(Token *tok, Type *typ) {
    Node *node = new_node(ND_DEFLOCAL, NULL, NULL);
    LVar *lvar = find_lvar(tok);
    if (lvar) {
        // エラー 定義済み
        error_at(tok->str, "定義済みの変数です。");
    }
    lvar = calloc(1, sizeof(LVar));
    lvar->next = locals;
    lvar->name = tok->str;
    lvar->len = tok->len;
    lvar->offset = (locals ? locals->offset : 0) + 8;
    lvar->type = typ;
    node->offset = lvar->offset;
    node->type = lvar->type;
    locals = lvar;
    return node;
}

// 変数名として識別
Node *new_node_lvar(Token *tok) {
    Node *node = new_node(ND_LVAR, NULL, NULL);
    LVar *lvar = find_lvar(tok);
    if (!lvar) {
        // エラー 未定義
        error_at(tok->str, "未定義の変数です。");
        return NULL;
    }
    node->offset = lvar->offset;
    node->type = lvar->type;
    return node;
}

// 文字列の先頭から変数名として読める部分の長さを取得
// [a-zA-Z_][a-zA-Z_0-9]*
int read_ident(char *p) {
    int i = 0;
    while (p[i] != '\0') {
        if ('a' <= p[i] && p[i] <= 'z' || 'A' <= p[i] && p[i] <= 'Z' ||
            p[i] == '_') {
            i++;
            continue;
        }
        if (0 < i) {
            if ('0' <= p[i] && p[i] <= '9') {
                i++;
                continue;
            }
        }
        break;
    }
    int len = i;
    return len;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char resv[][10] = {",", "{", "}", "<=", ">=", "==", "!=", "=", ";",  "+",
                       "-", "*", "/", "(",  ")",  "<",  ">",  "&", "int"};

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        bool is_tokenized = false;
        for (int i = 0; resv[i][0] != '\0'; i++) {
            int len = strlen(resv[i]);
            if (!memcmp(resv[i], p, len)) {
                cur = new_token(TK_RESERVED, cur, p, len);
                p += len;
                is_tokenized = true;
                break;
            }
        }
        if (is_tokenized) {
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 1);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        //先頭から変数として読める部分の長さを取得
        int ident_len = read_ident(p);
        // return 判定
        if (ident_len == 6 && !strncmp(p, "return", ident_len)) {
            cur = new_token(TK_CTRL, cur, p, ident_len);
            p += ident_len;
            continue;
        }

        // 制御構文if else while for等 判定
        char keyword[][8] = {"if", "else", "while", "for", ""};
        is_tokenized = false;
        for (int i = 0; keyword[i][0] != '\0'; i++) {
            if (ident_len == strlen(keyword[i]) &&
                !strncmp(p, keyword[i], ident_len)) {
                cur = new_token(TK_CTRL, cur, p, ident_len);
                p += ident_len;
                is_tokenized = true;
                break;
            }
        }
        if (is_tokenized) {
            continue;
        }

        // 変数名 判定
        if (ident_len > 0) {
            cur = new_token(TK_IDENT, cur, p, ident_len);
            p += ident_len;
            continue;
        }

        error_at(p, "トークナイズできません");
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

Node *code[100];
Node *statement[100];

Node *expr();

Node *primary() {
    // 次のトークンが"("なら、"(" expr ")" のはず
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // 変数名,関数名の識別
    Token *tok = consume_ident();
    if (!tok) {
        // 識別子がなければ数値のはず
        return new_node_num(expect_number());
    }

    if (!consume("(")) {
        // 変数名として識別
        return new_node_lvar(tok);
    }

    // 関数名として識別
    Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
    node->func_name = tok->str;
    node->func_name_len = tok->len;

    // 実引数処理
    if (!consume(")")) {
        node->next_arg = new_node(ND_FUNC_CALL_ARG, NULL, expr());
        node->next_arg->arg_idx = 0;
        Node *arg = node->next_arg;
        while (consume(",")) {
            arg->next_arg = new_node(ND_FUNC_CALL_ARG, NULL, expr());
            arg->next_arg->arg_idx = arg->arg_idx + 1;
            arg = arg->next_arg;
        }
        expect(")");
    }
    return node;
}

Node *unary() {
    if (consume("&")) {
        return new_node(ND_ADDR, unary(), NULL);
    }
    if (consume("*")) {
        return new_node(ND_DEREF, unary(), NULL);
    }
    if (consume("-")) {
        return new_node(ND_SUB, new_node_num(0), primary());
    }
    consume("+");
    return primary();
}

Node *mul() {
    Node *node = unary();
    for (;;) {
        if (consume("*")) {
            node = new_node(ND_MUL, node, unary());
        } else if (consume("/")) {
            node = new_node(ND_DIV, node, unary());
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

Type *type() {
    Type *typ = calloc(1, sizeof(Type));
    typ->ty = INT;
    while (consume("*")) {
        Type *ptr = calloc(1, sizeof(Type));
        ptr->ty = PTR;
        ptr->ptr_to = typ;
        typ = ptr;
    }
    return typ;
}

Node *declaration() {
    Type *typ = type();
    Token *tok = consume_ident();
    return new_node_deflocal(tok, typ);
}

Node *expr() { return assign(); }

// block {} である場合
Node *block() {
    if (!consume("{")) {
        return NULL;
    }
    Node *node = new_node(ND_BLOCK, NULL, NULL);
    node->block = calloc(128, sizeof(Node *));
    for (int i = 0; i <= 128; i++) {
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
    } else if (consume("return")) {
        node = new_node(ND_RETURN, expr(), NULL);
        expect(";");
    } else if (consume("if")) {
        expect("(");
        Node *judge = expr();
        expect(")");
        node = new_node(ND_IF_ELSE, stmt(), NULL);
        node->judge = judge;
        if (consume("else")) {
            node->rhs = stmt();
        }
        node->label_num = label_cnt;
        label_cnt++;
    } else if (consume("while")) {
        expect("(");
        Node *judge = expr();
        expect(")");
        node = new_node(ND_WHILE, stmt(), NULL);
        node->judge = judge;
        node->label_num = label_cnt;
        label_cnt++;
    } else if (consume("for")) {
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
        node->label_num = label_cnt;
        label_cnt++;
    } else if (consume("int")) {
        node = declaration();
        expect(";");
    } else {
        node = expr();
        expect(";");
    }
    return node;
}

// 関数定義ノード
Node *func() {
    expect("int");
    Func *fn = calloc(1, sizeof(Func));
    fn->next = funcs;
    fn->type = type();
    Token *tok = consume_ident();
    if (!tok) {
        free(fn);
        return NULL;  // エラー
    }

    //ローカル変数初期化
    locals = calloc(1, sizeof(LVar));

    Node *node = new_node(ND_FUNC_DEFINE, NULL, NULL);
    fn->name = tok->str;
    fn->len = tok->len;
    node->func_name = tok->str;
    node->func_name_len = tok->len;

    if (!consume("(")) {
        free(fn);
        return NULL;  // エラー
    }

    // 仮引数処理
    if (!consume(")")) {
        node->arg_idx = -1;
        Node *arg = node;
        do {
            consume("int");
            Node *ln = declaration();
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
    return node;
}

void program() {
    int i = 0;
    funcs = calloc(1, sizeof(Func));
    while (!at_eof()) {
        code[i] = func();
        i++;
    }
    code[i] = NULL;
}

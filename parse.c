#include "9cc.h"

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
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
    fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
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
    char resv[][3] = {",", "{", "}", "<=", ">=", "==", "!=", "=", ";",
                      "+", "-", "*", "/",  "(",  ")",  "<",  ">"};

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
    if (tok) {
        Node *node = calloc(1, sizeof(Node));

        // 関数名として識別
        if (consume("(")) {

            // 実引数処理
            if (!consume(")")) {
                node->kind = ND_ACTUAL_ARG;
                node->rhs = expr(); // 第一実引数の値
                node->arg_idx = 0;
                while (consume(",")) {
                    node = new_node(ND_ACTUAL_ARG, node, expr());
                    node->arg_idx = node->lhs->arg_idx + 1;
                }
                expect(")");
            }

            // 関数呼び出し本体
            node = new_node(ND_FUNC_CALL, node, NULL);
            node->func_name = tok->str;
            node->func_name_len = tok->len;
            return node;
        }

        // 変数名として識別
        node->kind = ND_LVAR;

        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar = calloc(1, sizeof(LVar));
            lvar->next = locals;
            lvar->name = tok->str;
            lvar->len = tok->len;
            lvar->offset = (locals ? locals->offset : 0) + 8;
            node->offset = lvar->offset;
            locals = lvar;
        }
        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}

Node *unary() {
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

Node *expr() { return assign(); }

Node *stmt() {
    Node *node;

    // block {} である場合
    if (consume("{")) {
        node = new_node(ND_BLOCK, NULL, NULL);
        node->block = calloc(128, sizeof(Node *));
        for (int i = 0; i <= 128; i++) {
            if (consume("}")) {
                break;
            }
            node->block[i] = stmt();
        }
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
    } else {
        node = expr();
        expect(";");
    }
    return node;
}

// 編集中
Node *func() {
    Token *tok = consume_ident();
    if (tok) {
        Node *node = calloc(1, sizeof(Node));

        // 関数名として識別
        if (consume("(")) {

            // 実引数処理
            if (!consume(")")) {
                // node->kind = ND_FORMAL_ARG;
                node->kind = ND_LVAR;
                LVar *lvar = find_lvar(tok);
                if (lvar) {
                    node->offset = lvar->offset;
                } else {
                    lvar = calloc(1, sizeof(LVar));
                    lvar->next = locals;
                    lvar->name = tok->str;
                    lvar->len = tok->len;
                    lvar->offset = (locals ? locals->offset : 0) + 8;
                    node->offset = lvar->offset;
                    locals = lvar;
                }
                node->arg_idx = 0;
                while (consume(",")) {
                    node = new_node(ND_FORMAL_ARG, node, expr());
                    node->arg_idx = node->lhs->arg_idx + 1;
                }
                expect(")");
            }

            // 関数呼び出し本体
            node = new_node(ND_FUNC_CALL, node, NULL);
            node->func_name = tok->str;
            node->func_name_len = tok->len;
            return node;
        }

        // 変数名として識別
        return node;
    }

    Node *node = calloc(1, sizeof(Node));
    for (;;) {
    }
    new_node(ND_FUNC_DEFINE, stmt(), NULL);
    int i = 0;
    return node;
}

void program() {
    int i = 0;
    while (!at_eof()) {
        // code[i] = func();
        code[i] = stmt();
        i++;
    }
    code[i] = NULL;
}

#include "9cc.h"

// Token の種類と予約語のペア構造体
typedef struct KindWordPair KindWordPair;
struct KindWordPair {
    TokenKind tokenkind;
    char word[10];
};

// 入力プログラム
char *user_input;
// 現在着目しているトークン
Token *token;

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
    char resv[][4] = {",", "{", "}", "<=", ">=", "==", "!=", "=", ";", "+",
                      "-", "*", "/", "(",  ")",  "<",  ">",  "&", "[", "]"};

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        // 行コメントをスキップ
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n') p++;
            continue;
        }

        // ブロックコメントをスキップ
        if (strncmp(p, "/*", 2) == 0) {
            char *q = strstr(p + 2, "*/");
            if (!q) error_at(p, "コメントが閉じられていません");
            p = q + 2;
            continue;
        }

        // 文字列判定
        if (*p == '"') {
            int len = 1;
            while (p[len++] != '"') {
            }
            cur = new_token(TK_STR, cur, p, len);
            p += len;
            continue;
        }

        for (int i = 0; i < sizeof(resv) / sizeof(resv[0]); i++) {
            int len = strlen(resv[i]);
            if (!memcmp(resv[i], p, len)) {
                cur = new_token(TK_RESERVED, cur, p, len);
                p += len;
                goto CONTINUE_WHILE_P;
            }
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 1);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        //先頭から変数として読める部分の長さを取得
        int ident_len = read_ident(p);

        // 制御構文if else while for等 判定
        KindWordPair kdwds[] = {{TK_CTRL, "if"},       {TK_CTRL, "else"},
                                {TK_CTRL, "while"},    {TK_CTRL, "for"},
                                {TK_RETURN, "return"}, {TK_SIZEOF, "sizeof"},
                                {TK_TYPE, "char"},     {TK_TYPE, "int"}};
        for (int i = 0; i < sizeof(kdwds) / sizeof(kdwds[0]); i++) {
            if (ident_len == strlen(kdwds[i].word) &&
                !strncmp(p, kdwds[i].word, ident_len)) {
                cur = new_token(kdwds[i].tokenkind, cur, p, ident_len);
                p += ident_len;
                goto CONTINUE_WHILE_P;
            }
        }

        // 変数名 判定
        if (ident_len > 0) {
            cur = new_token(TK_IDENT, cur, p, ident_len);
            p += ident_len;
            continue;
        }

        error_at(p, "トークナイズできません");
    CONTINUE_WHILE_P:;
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

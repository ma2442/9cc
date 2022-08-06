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

char escape(char c) {
    if (c == '0') return 0;   // NULL
    if (c == 'a') return 7;   // BEL
    if (c == 'b') return 8;   // BS (Back Space)
    if (c == 't') return 9;   // HT (Horizontal Tab)
    if (c == 'n') return 10;  // LF
    if (c == 'f') return 12;  // FF (Form Field)
    if (c == 'r') return 13;  // CR
    return c;
}

bool read_comment(char **pp) {
    char *p = *pp;
    // 行コメントをスキップ
    if (strncmp(p, "//", 2) == 0) {
        p += 2;
        while (*p != '\n') p++;
        *pp = p;
        return true;
    }
    // ブロックコメントをスキップ
    if (strncmp(p, "/*", 2) == 0) {
        char *q = strstr(p + 2, "*/");
        if (!q) error_at(p, "コメントが閉じられていません");
        *pp = q + 2;
        return true;
    }
    return false;
}

bool read_char(char **pp, Token **tokp) {
    char *p = *pp;
    if (*p == '\'') {
        if (p[1] == '\\') {
            *tokp = new_token(TK_CHAR, *tokp, &p[2], 1);
            (*tokp)->val = escape(p[2]);
            *pp = p + 4;
            return true;
        }
        *tokp = new_token(TK_CHAR, *tokp, &p[1], 1);
        (*tokp)->val = p[1];
        *pp = p + 3;
        return true;
    }
    return false;
}

bool read_str(char **pp, Token **tokp) {
    char *p = *pp;
    if (*p == '"') {
        int len = 1;
        while (p[len] != '"') {
            if (p[len] == '\\')
                len += 2;
            else
                len++;
        }
        len++;
        *tokp = new_token(TK_STR, *tokp, p, len);
        *pp = p + len;
        return true;
    }
    return false;
}

bool read_reserved(char **pp, Token **tokp) {
    const char resv[][4] = {"<<=", ">>=", "<<", ">>", "&&", "||", "++", "--",
                            "->",  "&=",  "^=", "|=", "+=", "-=", "*=", "/=",
                            "%=",  "<=",  ">=", "==", "!=", "=",  ".",  ",",
                            "{",   "}",   ";",  "+",  "-",  "*",  "/",  "%",
                            "(",   ")",   "<",  ">",  "&",  "!",  "?",  ":",
                            "[",   "]",   "|",  "^",  "~"};
    for (int i = 0; i < sizeof(resv) / sizeof(resv[0]); i++) {
        int len = strlen(resv[i]);
        if (!strncmp(resv[i], *pp, len)) {
            *tokp = new_token(TK_RESERVED, *tokp, *pp, len);
            *pp += len;
            return true;
        }
    }
    return false;
}

int read_numprefix(char **pp) {
    if (**pp == '0') {
        if ((*pp)[1] == 'x' || (*pp)[1] == 'X') {
            (*pp) += 2;
            return 16;
        } else if ((*pp)[1] == 'b' || (*pp)[1] == 'B') {
            (*pp) += 2;
            return 2;
        } else if (isdigit((*pp)[1])) {
            (*pp)++;
            return 8;
        }
    }
    return 10;
}

void read_numsuffix(char **pp, Token **tokp) {
    int cnt_lenlit = 0;
    int cnt_signlit = 0;
    TypeKind lenspec = INT;
    TypeKind signspec = SIGNED;
    char *p = *pp;
    while (cnt_lenlit < 2 && cnt_signlit < 2) {
        if (*p == 'l' || *p == 'L') {
            p++;
            lenspec = INT;
            if (*p == 'l' || *p == 'L') {
                p++;
                lenspec = LL;
            }
            cnt_lenlit++;
        } else if (*p == 'u' || *p == 'U') {
            p++;
            signspec = ~SIGNED;
            cnt_signlit++;
        } else {
            break;
        }
    }
    if (*pp == p) return;
    if (cnt_lenlit > 1 || cnt_signlit > 1)
        error_at(p - 1, "不正な定数接尾辞です");
    *tokp = new_token(TK_NUMSUFFIX, *tokp, *pp, p - *pp);
    (*tokp)->val = lenspec & signspec;
    *pp = p;
}

bool read_num(char **pp, Token **tokp) {
    if (!isdigit(**pp)) return false;
    *tokp = new_token(TK_NUM, *tokp, *pp, 1);
    int base = read_numprefix(pp);
    (*tokp)->val = strtoll(*pp, pp, base);
    (*tokp)->len = *pp - (*tokp)->str;
    read_numsuffix(pp, tokp);
    if (isdigit(**pp) || isalpha(**pp))
        error_at(*pp, "不正な%d進定数です", base);
    return true;
}

// 制御構文if else while for等 判定
bool read_controls(char **pp, Token **tokp, int len) {
    const KindWordPair kdwds[] = {{TK_CTRL, "if"},
                                  {TK_CTRL, "else"},
                                  {TK_CTRL, "switch"},
                                  {TK_CTRL, "case"},
                                  {TK_CTRL, "default"},
                                  {TK_CTRL, "break"},
                                  {TK_CTRL, "continue"},
                                  {TK_CTRL, "goto"},
                                  {TK_CTRL, "while"},
                                  {TK_CTRL, "do"},
                                  {TK_CTRL, "for"},
                                  {TK_CTRL, "typedef"},
                                  {TK_RETURN, "return"},
                                  {TK_SIZEOF, "sizeof"},
                                  {TK_TYPE, STR_VOID},
                                  {TK_TYPE, STR_INT},
                                  {TK_TYPE, STR_CHAR},
                                  {TK_TYPE, STR_BOOL},
                                  {TK_TYPEQ_SIGN, STR_SIGNED},
                                  {TK_TYPEQ_SIGN, STR_UNSIGNED},
                                  {TK_TYPEQ_LENGTH, STR_LONG},
                                  {TK_TYPEQ_LENGTH, STR_SHORT},
                                  {TK_STRUCT, "struct"},
                                  {TK_ENUM, "enum"}};
    for (int i = 0; i < sizeof(kdwds) / sizeof(kdwds[0]); i++) {
        if (len == strlen(kdwds[i].word) && !strncmp(*pp, kdwds[i].word, len)) {
            *tokp = new_token(kdwds[i].tokenkind, *tokp, *pp, len);
            *pp += len;
            return true;
        }
    }
    return false;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (read_comment(&p)) continue;
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;

        //先頭から変数として読める部分の長さを取得
        int ident_len = read_ident(p);

        if (read_controls(&p, &cur, ident_len)) continue;

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

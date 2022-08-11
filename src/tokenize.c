#include "9cc_manual.h"

Def *dreplace;      // #define リスト
bool skip = false;  // プリプロセスのif失敗中

Def *find_replace(Token *idt) {
    for (Def *d = dreplace; d; d = d->next) {
        if (sametok(d->tok, idt)) return d;
    }
    return NULL;
}

// Token の種類と予約語のペア構造体
typedef struct KindWordPair KindWordPair;
struct KindWordPair {
    TokenKind tokenkind;
    char *word;
};

Token *new_tok(TokenKind kind, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    return tok;
}

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
    if (c == 'v') return 11;  // VT (Vertical Tab)
    if (c == 'f') return 12;  // FF (Form Field)
    if (c == 'r') return 13;  // CR
    return c;
}

bool read_comment(char **pp) {
    char *p = *pp;
    // 行コメントをスキップ
    if (strncmp(p, "//", 2) == MATCH) {
        p += 2;
        while (*p != '\n') p++;
        *pp = p;
        return true;
    }
    // ブロックコメントをスキップ
    if (strncmp(p, "/*", 2) == MATCH) {
        char *q = strstr(p + 2, "*/");
        if (!q) error_at(p, ERRNO_TOKENIZE_COMMENT);
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
    int rlen = 0;
    char *resv[100];
    resv[rlen++] = "...";
    resv[rlen++] = "<<=";
    resv[rlen++] = ">>=";
    resv[rlen++] = "<<";
    resv[rlen++] = ">>";
    resv[rlen++] = "&&";
    resv[rlen++] = "||";
    resv[rlen++] = "++";
    resv[rlen++] = "--";
    resv[rlen++] = "->";
    resv[rlen++] = "&=";
    resv[rlen++] = "^=";
    resv[rlen++] = "|=";
    resv[rlen++] = "+=";
    resv[rlen++] = "-=";
    resv[rlen++] = "*=";
    resv[rlen++] = "/=";
    resv[rlen++] = "%=";
    resv[rlen++] = "<=";
    resv[rlen++] = ">=";
    resv[rlen++] = "==";
    resv[rlen++] = "!=";
    resv[rlen++] = "=";
    resv[rlen++] = ".";
    resv[rlen++] = ",";
    resv[rlen++] = "{";
    resv[rlen++] = "}";
    resv[rlen++] = ";";
    resv[rlen++] = "+";
    resv[rlen++] = "-";
    resv[rlen++] = "*";
    resv[rlen++] = "/";
    resv[rlen++] = "%";
    resv[rlen++] = "(";
    resv[rlen++] = ")";
    resv[rlen++] = "<";
    resv[rlen++] = ">";
    resv[rlen++] = "&";
    resv[rlen++] = "!";
    resv[rlen++] = "?";
    resv[rlen++] = ":";
    resv[rlen++] = "[";
    resv[rlen++] = "]";
    resv[rlen++] = "|";
    resv[rlen++] = "^";
    resv[rlen++] = "~";
    for (int i = 0; i < rlen; i++) {
        int len = strlen(resv[i]);
        if (strncmp(resv[i], *pp, len) == MATCH) {
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
    bool is_unsigned = false;
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
            is_unsigned = true;
            cnt_signlit++;
        } else {
            break;
        }
    }
    if (*pp == p) return;
    if (cnt_lenlit > 1 || cnt_signlit > 1)
        error_at(p - 1, ERRNO_TOKENIZE_NUMSUFFIX);
    *tokp = new_token(TK_NUMSUFFIX, *tokp, *pp, p - *pp);
    (*tokp)->val = is_unsigned ? lenspec & ~SIGNED : lenspec;
    *pp = p;
}

bool read_num(char **pp, Token **tokp) {
    if (!isdigit(**pp)) return false;
    *tokp = new_token(TK_NUM, *tokp, *pp, 1);
    int base = read_numprefix(pp);
    (*tokp)->val = strtoll(*pp, pp, base);
    (*tokp)->len = *pp - (*tokp)->str;
    read_numsuffix(pp, tokp);
    if (isdigit(**pp) || isalpha(**pp)) {
        error_at(*pp, ERRNO_TOKENIZE_CONST);
    }
    return true;
}

// 制御構文if else while for等 判定
bool read_controls(char **pp, Token **tokp, int len) {
    int klen = 0;
    KindWordPair kdwds[100];
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "if";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "else";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "switch";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "case";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "default";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "break";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "continue";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "goto";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "while";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "do";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "for";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "typedef";
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "extern";
    kdwds[klen].tokenkind = TK_RETURN;
    kdwds[klen++].word = "return";
    kdwds[klen].tokenkind = TK_SIZEOF;
    kdwds[klen++].word = "sizeof";
    kdwds[klen].tokenkind = TK_TYPE;
    kdwds[klen++].word = STR_VOID;
    kdwds[klen].tokenkind = TK_TYPE;
    kdwds[klen++].word = STR_INT;
    kdwds[klen].tokenkind = TK_TYPE;
    kdwds[klen++].word = STR_CHAR;
    kdwds[klen].tokenkind = TK_TYPE;
    kdwds[klen++].word = STR_BOOL;
    kdwds[klen].tokenkind = TK_TYPEQ_SIGN;
    kdwds[klen++].word = STR_SIGNED;
    kdwds[klen].tokenkind = TK_TYPEQ_SIGN;
    kdwds[klen++].word = STR_UNSIGNED;
    kdwds[klen].tokenkind = TK_TYPEQ_LENGTH;
    kdwds[klen++].word = STR_LONG;
    kdwds[klen].tokenkind = TK_TYPEQ_LENGTH;
    kdwds[klen++].word = STR_SHORT;
    kdwds[klen].tokenkind = TK_STRUCT;
    kdwds[klen++].word = "struct";
    kdwds[klen].tokenkind = TK_ENUM;
    kdwds[klen++].word = "enum";

    for (int i = 0; i < klen; i++) {
        if (len == strlen(kdwds[i].word) &&
            strncmp(*pp, kdwds[i].word, len) == MATCH) {
            *tokp = new_token(kdwds[i].tokenkind, *tokp, *pp, len);
            *pp += len;
            return true;
        }
    }
    return false;
}

bool skip_nontoken_notLF(char **pp) {
    for (bool done = false;; done = true) {
        if (**pp != '\n' && isspace(**pp))
            (*pp)++;
        else if (read_comment(pp))
            ;
        else
            return done;
    }
}

char *read_innner_strlike(char **pp, char begin, char end) {
    char *p = *pp;
    if (*p == begin) {
        int len = 1;
        while (p[len] != end) {
            if (p[len] == '\\')
                len += 2;
            else
                len++;
        }
        len++;
        *pp = p + len;
        char *str = calloc(256, sizeof(char));
        strncpy(str, p + 1, len - 2);
        return str;
    }
    return NULL;
}

bool read_match(char **pp, char *str, int len) {
    if (len == strlen(str) && strncmp(*pp, str, len) == MATCH) {
        *pp += len;
        return true;
    }
    return false;
}

char *make_abspath(char **pp, char *dir) {
    char *path = read_innner_strlike(pp, '"', '"');
    if (!path) return NULL;
    char *abs = strcat(cpy_dirname(dir), path);
    int len = strlen(abs);
    char *rev = calloc(len + 2, sizeof(char));
    int ir = 0;
    int skip_cnt = 0;
    int i = len;
    while (i >= 0) {
        int ichk = i - 3;
        if (ichk >= 0 && strncmp(abs + ichk, "/../", 4) == MATCH) {
            skip_cnt++;
            i = ichk;
            continue;
        }
        if (skip_cnt) {
            i--;
            while (i >= 0 && abs[i] != '/') i--;
            skip_cnt--;
            continue;
        }
        rev[ir] = abs[i];
        ir++;
        i--;
    }
    for (; ir >= 0; ir--) {
        abs[i] = rev[ir];
        i++;
    }

    return abs;
}

// #define IDENT ________ のアンダーライン部分をトークナイズ
Token *tokenize_predefine(char *p) {
    Token head;
    head.next = NULL;
    Token *tok_inc = NULL;
    Token *cur = &head;
    while (*p != '\n') {
        if (skip_nontoken_notLF(&p)) continue;
        // 通常のトークン読み
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;
        //先頭から変数として読める部分の長さを取得
        int idtlen = read_ident(p);
        if (read_controls(&p, &cur, idtlen)) continue;
        // 変数名 判定
        if (idtlen > 0) {
            cur = new_token(TK_IDENT, cur, p, idtlen);
            p += idtlen;
            continue;
        }
        error_at(p, ERRNO_TOKENIZE);
    }
    return head.next;
}

Token *cpy_alltoken(Token *from) {
    Token head;
    head.next = NULL;
    Token *to = &head;
    while (from) {
        to->next = calloc(1, sizeof(Token));
        *to->next = *from;
        from = from->next;
        to = to->next;
    }
    return head.next;
}

bool preproc(char **pp, Token **tokp, char *filepath) {
    char *p = *pp;
    Token *cur = *tokp;
    if (*p != '#') {
        if (!skip)
            return false;
        else
            goto END;
    }
    p++;  // '#' の直後
    skip_nontoken_notLF(&p);
    // ディレクティブの長さ取得
    int drctlen = read_ident(p);
    if (read_match(&p, "else", drctlen)) {
        skip = !skip;
    } else if (skip) {
        if (read_match(&p, "endif", drctlen)) {
            skip = false;
        }
    } else if (read_match(&p, "include", drctlen)) {
        skip_nontoken_notLF(&p);
        char *incpath = make_abspath(&p, filepath);
        char *content = read_file(incpath);
        cur->next = tokenize(content, incpath);
        while (cur->next && cur->next->kind != TK_EOF) cur = cur->next;
    } else if (read_match(&p, "define", drctlen)) {
        skip_nontoken_notLF(&p);
        int len = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, len);
        if (find_replace(idt)) error_at(p, ERRNO_PREPROC_DEF);
        Def *drep = calloc(1, sizeof(Def));
        drep->tok = new_tok(TK_IDENT, p, len);
        p += len;
        drep->replace = tokenize_predefine(p);
        drep->next = dreplace;
        dreplace = drep;
    } else if (read_match(&p, "ifdef", drctlen) ||
               read_match(&p, "ifndef", drctlen)) {
        skip_nontoken_notLF(&p);
        int len = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, len);
        if (!find_replace(idt)) skip = true;
        if (drctlen == 6) skip = !skip;  // ifndef
    } else {
        // ディレクティブでない 読み飛ばす
    }
END:
    while (*p != '\n') p++;
    *pp = p;
    *tokp = cur;
    return true;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p, char *filepath) {
    Token head;
    head.next = NULL;
    Token *tok_inc = NULL;
    Token *cur = &head;
    bool is_linehead = true;
    while (*p) {
        if (skip_nontoken_notLF(&p)) continue;
        if (*p == '\n') {
            p++;
            is_linehead = true;
            continue;
        }
        // プリプロセス
        if (is_linehead && preproc(&p, &cur, filepath)) continue;

        is_linehead = false;
        if (skip) continue;
        // 通常のトークン読み
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;

        //先頭から変数として読める部分の長さを取得
        int idtlen = read_ident(p);
        // #define で定義された識別子を置き換え
        Token *idt = new_tok(TK_IDENT, p, idtlen);
        Def *drep = find_replace(idt);
        if (drep) {
            cur->next = cpy_alltoken(drep->replace);
            while (cur->next) cur = cur->next;
            p += idtlen;
            continue;
        }
        if (read_controls(&p, &cur, idtlen)) continue;

        // 変数名 判定
        if (idtlen > 0) {
            cur = new_token(TK_IDENT, cur, p, idtlen);
            p += idtlen;
            continue;
        }

        error_at(p, ERRNO_TOKENIZE);
    }

    new_token(TK_EOF, cur, p, 1);
    return head.next;
}

#include "9cc_manual.h"

bool read_macro(char **pp, Token **tokp);
typedef struct Macro Macro;
// #define macro
struct Macro {
    Macro *next;
    Token *tok;
    Token **params;  // パラメタリスト
    int pcnt;        // パラメタ数
    Token **ctts;    // 内容リスト
    int ccnt;        // 内容 トークン数
};

Macro *macro = NULL;  // #define リスト
bool skip = false;    // プリプロセスのif失敗中
int nestif = 0;       // #if のネスト
int nestskip = -1;    // skip中のifネスト

Macro *find_macro(Token *idt) {
    for (Macro *m = macro; m; m = m->next) {
        if (sametok(m->tok, idt)) return m;
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

bool read_merge(char **pp, Token **tokp) {
    if ((*pp)[0] == '#' && (*pp)[1] == '#') {
        *tokp = new_token(TK_MERGE, *tokp, (*pp), 2);
        (*pp) += 2;
        return true;
    }
    return false;
}

bool read_stringize(char **pp, Token **tokp) {
    if ((*pp)[0] == '#') {
        *tokp = new_token(TK_STRINGIZE, *tokp, (*pp), 1);
        (*pp) += 1;
        return true;
    }
    return false;
}

bool skip_nontoken_except(char **pp, char excpt) {
    for (bool done = false;; done = true) {
        if (**pp == '\\') {
            (*pp)++;
            while (**pp != '\n' && isspace(**pp)) (*pp)++;
            if (**pp == '\n')
                (*pp)++;
            else
                return true;
        } else if (**pp != excpt && isspace(**pp))
            (*pp)++;
        else if (read_comment(pp))
            ;
        else
            return done;
    }
}

bool skip_nontoken(char **pp) { return skip_nontoken_except(pp, '\0'); }
bool skip_nontoken_notLF(char **pp) { return skip_nontoken_except(pp, '\n'); }

char *read_inner_strlike(char **pp, char begin, char end) {
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
    bool is_curdir = true;
    char *path = read_inner_strlike(pp, '"', '"');
    if (!path) {
        path = read_inner_strlike(pp, '<', '>');
        is_curdir = false;
    }
    if (!path) return NULL;
    char *abs;
    if (is_curdir) {
        abs = strcat(cpy_dirname(dir), path);
    } else {
        abs = strcat(cpy_dirname("/usr/include/"), path);
        if (!fopen(abs, "r"))
            abs = strcat(cpy_dirname("/usr/include/x86_64-linux-gnu/"), path);
        if (!fopen(abs, "r"))
            abs = strcat(
                cpy_dirname("/usr/lib/gcc/x86_64-linux-gnu/9/include/"), path);
    }
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

Token *try_tokenize_next(char **pp) {
    char *p = *pp;
    skip_nontoken_notLF(&p);
    Token head;
    Token *cur = &head;
    do {
        // if (read_merge(&p, &cur)) break;
        if (read_str(&p, &cur)) break;
        if (read_char(&p, &cur)) break;
        if (read_reserved(&p, &cur)) break;
        if (read_num(&p, &cur)) break;
        if (read_macro(&p, &cur)) break;
        //先頭から変数として読める部分の長さを取得
        int idtlen = read_ident(p);
        if (read_controls(&p, &cur, idtlen)) break;
        // 変数名 判定
        if (idtlen > 0) {
            cur = new_token(TK_IDENT, cur, p, idtlen);
            p += idtlen;
            break;
        }
    } while (0);
    *pp = p;
    return head.next;
}

Token *find_param(Token *idt, Token **params, int pcnt) {
    for (int ip = 0; ip < pcnt; ip++) {
        if (sametok(params[ip], idt)) return params[ip];
    }
    return NULL;
}

Macro *make_macro_from_token(Token *tok, Token **params, int pcnt) {
    Macro *mcr = calloc(1, sizeof(Macro));
    int ccnt = 0;
    Token *save = tok;
    while (tok) {
        tok = tok->next;
        ccnt++;
    }
    tok = save;
    mcr->pcnt = pcnt;
    mcr->params = params;
    Token **ctts = calloc(ccnt, sizeof(Token *));
    for (int i = 0; i < ccnt; i++) {
        // パラメタと同じトークンならパラメータのポインタを入れる
        Token *par = find_param(tok, params, pcnt);
        ctts[i] = tok;
        if (par) ctts[i] = par;
        tok = tok->next;
    }
    mcr->ctts = ctts;
    mcr->ccnt = ccnt;
    return mcr;
}

Token *make_token_from_macro(Macro *mcr) {
    if (mcr->ccnt <= 0) return NULL;
    Token **ctts = calloc(mcr->ccnt, sizeof(Token *));
    int ccnt = 0;
    char buf[1000];
    int bcnt = 0;
    Token *pre = NULL;
    Token *cur = mcr->ctts[0];
    for (int i = 1; i < mcr->ccnt; i++) {
        pre = cur;
        cur = mcr->ctts[i];
        if (cur->kind == TK_MERGE && !bcnt) {  // not(##) ##
            strncpy(buf + bcnt, pre->str, pre->len);
            bcnt += pre->len;
        }
        if (pre->kind == TK_MERGE) {  // ## not(##)
            strncpy(buf + bcnt, cur->str, cur->len);
            bcnt += cur->len;
        }
        if (pre->kind != TK_MERGE && cur->kind != TK_MERGE ||
            i == mcr->ccnt - 1) {
            //  not(##) not(##)
            if (bcnt) {
                buf[bcnt] = '\0';
                char *p = calloc(bcnt + 1, sizeof(char));
                strncpy(p, buf, bcnt + 1);
                ctts[ccnt] = tokenize(p, NULL);
                bcnt = 0;
            } else {
                ctts[ccnt] = calloc(1, sizeof(Token));
                *ctts[ccnt] = *pre;
            }
            if (!eqtokstr(ctts[ccnt], "")) ccnt++;
        }
    }

    if (!pre || pre->kind != TK_MERGE && cur->kind != TK_MERGE) {
        ctts[ccnt] = calloc(1, sizeof(Token));
        *ctts[ccnt] = *cur;
        if (!eqtokstr(ctts[ccnt], "")) ccnt++;
    }

    Token head;
    cur = &head;
    for (int i = 0; i < ccnt; i++) {
        cur->next = ctts[i];
        cur = cur->next;
    }
    cur->next = NULL;
    return head.next;
}

// #define IDENT ________ のアンダーライン部分をトークナイズ
Token *tokenize_macro_ctts(char *p, Token **params, int pcnt) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p != '\n') {
        if (skip_nontoken_notLF(&p)) continue;
        if (read_merge(&p, &cur)) continue;
        if (read_stringize(&p, &cur)) continue;
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;

        int len = read_ident(p);
        // パラメータと同じ識別子ならマクロ展開をしない
        // 前後のトークンどちらかが##ならマクロ展開しない
        Token *idt = new_tok(TK_IDENT, p, len);
        Token *cur1 = cur;
        char *p1 = p;
        if (!find_param(idt, params, pcnt) && cur->kind != TK_MERGE &&
            read_macro(&p1, &cur1)) {
            char *tmp = p1;
            Token *mrg = try_tokenize_next(&tmp);
            if (mrg && mrg->kind == TK_MERGE) return false;
            p = p1;
            cur = cur1;
            continue;
        }
        if (read_controls(&p, &cur, len)) continue;
        // 変数名 判定
        if (len > 0) {
            cur = new_token(TK_IDENT, cur, p, len);
            p += len;
            continue;
        }
        error_at(p, ERRNO_TOKENIZE);
    }
    return head.next;
}

// #if ________ のアンダーライン部分をトークナイズ
Token *tokenize_macro_if(char *p) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p != '\n') {
        if (skip_nontoken_notLF(&p)) continue;
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;

        int idtlen = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, idtlen);
        if (eqtokstr(idt, "defined")) {
            p += idtlen;
            skip_nontoken_notLF(&p);
            bool inner = false;
            if (*p == '(') {
                p++;
                inner = true;
                skip_nontoken_notLF(&p);
            }
            int len = read_ident(p);
            Token *mcridt = new_tok(TK_IDENT, p, len);
            cur = new_token(TK_NUM, cur, p, 1);
            cur->val = find_macro(mcridt) ? 1 : 0;
            p += len;
            if (inner && *p == ')') p++;
            continue;
        }
        if (read_macro(&p, &cur)) continue;
        if (idtlen > 0) {
            // 未定義のマクロは0として評価する
            cur = new_token(TK_NUM, cur, "0", 1);
            cur->val = 0;
            p += idtlen;
            continue;
        }
        error_at(p, ERRNO_TOKENIZE);
    }
    return head.next;
}

Token *tokenize_param(char **pp) {
    char *p = *pp;
    Token head;
    Token *cur = &head;
    int inner = 0;
    while (inner || !eqtokstr(cur, ",") && !eqtokstr(cur, ")")) {
        if (eqtokstr(cur, "(")) {
            inner++;
        } else if (inner && eqtokstr(cur, ")")) {
            inner--;
        }
        cur->next = try_tokenize_next(&p);
        if (!cur->next) error_at(p, ERRNO_TOKENIZE);
        cur = cur->next;
    }
    cur->str = "";  // 最後の"," or ")" を "" に変更
    cur->len = 0;
    *pp = p - 1;  // ',' or ')'
    return head.next;
}

bool read_macro(char **pp, Token **tokp) {
    char *p = *pp;
    Token *cur = *tokp;
    int len = read_ident(p);
    Token *idt = new_tok(TK_IDENT, p, len);
    p += len;
    Macro *mcr = find_macro(idt);
    if (!mcr) return false;

    Token **params = mcr->params;
    if (mcr->pcnt == -1) {  // パラメタを持たないマクロ
    } else {
        skip_nontoken(&p);
        if (*p != '(') return false;
        // マクロの識別子の後のパラメタ部分(..)を読む
        int pcnt = 0;
        do {
            p++;  // '(' or ','
            Token *prm = tokenize_param(&p);
            if (prm) *params[pcnt] = *prm;
            pcnt++;
        } while (*p != ')');
        p++;
        if (pcnt == 1 && params[0]->len == 0) pcnt = 0;
        if (!(mcr->pcnt == 1 && pcnt == 0) && mcr->pcnt != pcnt)
            error_at(p, ERRNO_PREPROC_PARAMCNT);
    }
    cur->next = make_token_from_macro(mcr);
    while (cur->next) cur = cur->next;
    *pp = p;
    *tokp = cur;
    return true;
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
    if (read_match(&p, "ifdef", drctlen) || read_match(&p, "ifndef", drctlen)) {
        nestif++;
        if (!skip) {
            skip_nontoken_notLF(&p);
            int len = read_ident(p);
            Token *idt = new_tok(TK_IDENT, p, len);
            if (!find_macro(idt)) skip = true;
            if (drctlen == 6) skip = !skip;  // ifndef
            if (skip) nestskip == nestif;
            nestskip = nestif;
        }
    } else if (read_match(&p, "if", drctlen)) {
        nestif++;
        if (!skip) {
            Token *save = token;
            token = tokenize_macro_if(p);
            if (!val(expr())) skip = true;
            token = save;
            if (skip) nestskip == nestif;
            nestskip = nestif;
        }
    } else if (read_match(&p, "endif", drctlen)) {
        if (nestskip == nestif) skip = false;
        nestif--;
    } else if (read_match(&p, "else", drctlen)) {
        if (nestskip == nestif) skip = !skip;
    } else if (read_match(&p, "elif", drctlen)) {
        if (nestskip == nestif && skip) {
            Token *save = token;
            token = tokenize_macro_if(p);
            if (val(expr())) skip = false;
            token = save;
        }
    } else if (skip) {
    } else if (read_match(&p, "include", drctlen)) {
        skip_nontoken_notLF(&p);
        char *incpath = make_abspath(&p, filepath);
        char *content = read_file(incpath);
        cur->next = tokenize(content, incpath);
        while (cur->next && cur->next->kind != TK_EOF) cur = cur->next;
    } else if (read_match(&p, "undef", drctlen)) {
        skip_nontoken_notLF(&p);
        int len = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, len);
        Macro *mcr = find_macro(idt);
        if (mcr) mcr->tok = NULL;
    } else if (read_match(&p, "define", drctlen)) {
        skip_nontoken_notLF(&p);
        int len = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, len);
        if (find_macro(idt)) error_at(p, ERRNO_PREPROC_DEF);
        p += len;
        // マクロの識別子の後のパラメタ部分(..)を読む
        Token *ps[100];
        Token **params = NULL;
        int pcnt = -1;
        if (*p == '(') {
            pcnt++;
            do {
                p++;
                skip_nontoken_notLF(&p);
                int plen = read_ident(p);
                if (!plen) break;
                ps[pcnt] = new_tok(TK_IDENT, p, plen);
                p += plen;
                pcnt++;
                skip_nontoken_notLF(&p);
            } while (*p == ',');
            skip_nontoken_notLF(&p);
            if (*p != ')') error_at(p, ERRNO_TOKENIZE);
            p++;
        }
        if (pcnt > 0) {
            params = calloc(pcnt, sizeof(Token *));
            for (int i = 0; i < pcnt; i++) {
                params[i] = ps[i];
            }
        } else {
            params = calloc(1, sizeof(Token *));
            params[0] = new_tok(TK_IDENT, "", 0);
        }
        Token *ctts = tokenize_macro_ctts(p, params, pcnt);
        Macro *mcr = make_macro_from_token(ctts, params, pcnt);
        mcr->tok = idt;
        mcr->next = macro;
        macro = mcr;
    } else {
        // ディレクティブでない 読み飛ばす
    }
END:
    while (*p != '\n') {  // 行送り
        if (skip_nontoken_notLF(&p)) continue;
        p++;
    }
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
        // #define で定義された識別子を置き換え
        if (read_macro(&p, &cur)) continue;
        //先頭から変数として読める部分の長さを取得
        int idtlen = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, idtlen);
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

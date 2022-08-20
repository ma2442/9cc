#include "9cc.h"

Token *pp_macro();
typedef struct Macro Macro;
// #define macro
struct Macro {
    Macro *next;
    Token *tok;
    Token **params;      // パラメタリスト
    int pcnt;            // パラメタ数
    Token **body;        // 内容リスト
    int bcnt;            // 内容 トークン数
    bool forbid_expand;  // 展開禁止か
};

Macro *macro = NULL;  // #define リスト
bool skip = false;    // プリプロセスのif失敗中
int nestif = 0;       // #if のネスト
int nestskip = -1;    // skip中のifネスト

// 展開可能なマクロを探す
// マクロ展開中は自分自身を再帰展開不可
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
    tok->has_space = false;
    tok->need_merge = false;
    tok->is_linehead = false;
    tok->forbid_expand = false;
    return tok;
}

// 新しいトークンを作成してcurに繋げる
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    tok->has_space = false;
    tok->need_merge = false;
    tok->is_linehead = eqtokstr(cur, "\n");
    tok->forbid_expand = false;
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

bool read_default_path(char **pp, Token **tokp) {
    char *p = *pp;
    if (*p == '<') {
        int len = 1;
        while (p[len] != '>') len++;
        len++;
        *tokp = new_token(TK_DEFAULT_PATH, *tokp, p, len);
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
    resv[rlen++] = "##";
    resv[rlen++] = "#";
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
    resv[rlen++] = "\n";
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
bool read_controls(char **pp, Token **tokp, Token *idt) {
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
    kdwds[klen].tokenkind = TK_CTRL;
    kdwds[klen++].word = "static";
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
    kdwds[klen].tokenkind = TK_UNION;
    kdwds[klen++].word = "union";
    kdwds[klen].tokenkind = TK_ENUM;
    kdwds[klen++].word = "enum";

    for (int i = 0; i < klen; i++) {
        if (eqtokstr(idt, kdwds[i].word)) {
            *tokp = new_token(kdwds[i].tokenkind, *tokp, *pp, idt->len);
            *pp += idt->len;
            return true;
        }
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

void newline() {
    while (token && !consume("\n")) token = token->next;
}

// <..> または ".." のパスの内容を読んでプリプロセッサにかけた
// トークンをtokenの前につなげる関数
// ついでにtokenの最初に"\n"トークンを挿入する
Token *pp_include(char *dir) {
    if (token->kind != TK_STR && token->kind != TK_DEFAULT_PATH) return NULL;
    int pathlen = token->len - 2;
    char *path = calloc(pathlen + 1, sizeof(char));
    strncpy(path, token->str + 1, pathlen);  // 両端 "" <> を除外
    path[pathlen] = '\0';
    char *abs;
    if (token->kind == TK_STR) {
        if (path[0] == '/')  // 絶対パス表記
            abs = path;
        else  // 相対パス表記
            abs = strcat(cpy_dirname(dir), path);
    } else {
        abs = strcat(cpy_dirname("/usr/include/"), path);
        if (!fopen(abs, "r"))
            abs = strcat(cpy_dirname("/usr/include/x86_64-linux-gnu/"), path);
        if (!fopen(abs, "r"))
            abs = strcat(
                cpy_dirname("/usr/lib/gcc/x86_64-linux-gnu/9/include/"), path);
    }
    newline();
#ifdef __DEBUG__READ__FILE__
    fprintf(stderr, "open %s\n", path);
#endif
    char *included = read_file(abs);
    Token head;
    Token *cur = &head;
    cur = new_token(TK_RESERVED, cur, "\n", 1);
    cur->next = preproc(tokenize(included), abs);
    while (cur->next && cur->next->kind != TK_EOF) {
        cur = cur->next;
        cur->forbid_expand = true;
    }
    cur->next = token;
    token = head.next;
#ifdef __DEBUG__READ__FILE__
    fprintf(stderr, "finish tokenizing and preprocessing %s\n", path);
#endif
    return head.next;
}

Token *find_param(Token *idt, Token **params, int pcnt) {
    for (int ip = 0; ip < pcnt; ip++) {
        if (sametok(params[ip], idt)) return params[ip];
    }
    return NULL;
}

Token *cpy_tokens(Token *tok) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    for (Token *t = tok; t; t = t->next) {
        if (t->str[0] == '\0') {
            cur->next = NULL;
            continue;
        }
        cur->next = calloc(1, sizeof(Token));
        *cur->next = *t;
        cur = cur->next;
    }
    return head.next;
}

Token *prepare_merge(Token *tok) {
    Token head;
    head.next = NULL;
    Token *cur = &head;
    for (Token *t = tok; t; t = t->next) {
        if (t->str[0] == '\0') {
            cur->next = NULL;
            continue;
        }
        if (eqtokstr(t, "##")) {
            cur->need_merge = true;
            continue;
        }
        cur->next = t;
        cur = cur->next;
    }
    return head.next;
}

Token *merge_tokens(Token *tok) {
    tok = prepare_merge(tok);
    Token head;
    head.next = NULL;
    Token *cur = &head;
    char buf[1000];
    int len = 0;
    for (Token *t = tok; t; t = t->next) {
        if (t->need_merge) {
            strncpy(buf + len, t->str, t->len);
            len += t->len;
        } else if (len) {
            strncpy(buf + len, t->str, t->len);
            len += t->len;
            buf[len] = '\0';
            char *str = calloc(len + 1, sizeof(char));
            strncpy(str, buf, len + 1);
            len = 0;
            cur->next = tokenize(str);
            while (cur->next && cur->next->kind != TK_EOF) cur = cur->next;
            cur->next = NULL;
        } else {
            cur->next = t;
            cur = cur->next;
        }
    }
    return head.next;
}

Token *make_token_from_macro(Macro *mcr) {
    if (mcr->bcnt <= 0) return NULL;
    Token head;
    Token *cur = &head;
    for (int i = 0; i < mcr->bcnt; i++) {
        cur->next = cpy_tokens(mcr->body[i]);
        while (cur->next) cur = cur->next;
    }
    return head.next;
}

// #define  ___ ________ のアンダーライン部分を読んでマクロ作成
// パラメータのみアドレスコピー
// それ以外は実態コピー
Macro *pp_define() {
    Token *idt = consume_identpp();
    // if (find_macro(idt)) error_at(p, ERRNO_PREPROC_DEF);
    Macro *fnd = find_macro(idt);
    if (fnd) fnd->tok = NULL;
    // マクロの識別子の後のパラメタ部分(..)を読む
    Token *params[100];
    int pcnt = -1;
    if (!idt->has_space && consume("(")) {
        pcnt++;
        do {
            Token *ptok = consume_identpp();
            if (!ptok) break;
            params[pcnt] = calloc(1, sizeof(Token));
            *params[pcnt] = *ptok;
            params[pcnt]->next = NULL;
            pcnt++;
        } while (consume(","));
        expect(")");
    }

    // マクロ本体を読む
    int bcnt = 0;
    Token *body[1000];
    for (; !current_is("\n"); token = token->next) {
        Token *prm = find_param(token, params, pcnt);
        if (prm) {
            body[bcnt++] = prm;
            continue;
        }
        body[bcnt] = calloc(1, sizeof(Token));
        *body[bcnt] = *token;
        body[bcnt++]->next = NULL;
    }

    // マクロ作成
    Macro *mcr = calloc(1, sizeof(Macro));
    mcr->tok = idt;
    mcr->body = calloc(bcnt, sizeof(Token *));
    for (int i = 0; i < bcnt; i++) mcr->body[i] = body[i];
    mcr->bcnt = bcnt;
    mcr->params = calloc(pcnt, sizeof(Token *));
    for (int i = 0; i < pcnt; i++) mcr->params[i] = params[i];
    mcr->pcnt = pcnt;
    mcr->forbid_expand = false;
    return mcr;
}

// #if ________ のアンダーライン部分をトークナイズ, 評価
bool pp_if() {
    Token head;
    Token *cur = &head;
    while (!current_is("\n")) {
        if (consume("defined")) {  // defined (x) or defined x
            bool inner = false;
            if (consume("(")) inner = true;
            Token *idt = consume_identpp();
            cur = new_token(TK_NUM, cur, idt->str, 1);
            cur->val = find_macro(idt) ? 1 : 0;
            if (inner) consume(")");
            continue;
        }
        // 定義済みマクロ展開
        Token *mtok = pp_macro();
        if (mtok) {
            cur->next = mtok;
            continue;
        }

        // 定義済みマクロ以外
        Token *idt = consume_identpp();
        // 未定義のマクロは0として評価する
        if (idt && !find_macro(idt)) {
            cur = new_token(TK_NUM, cur, "0", 1);
            cur->val = 0;
            continue;
        }

        // 識別子以外
        cur->next = token;
        cur = cur->next;
        token = token->next;
        continue;
    }
    Token *save = token;
    token = head.next;
    bool ret = val(expr());
    token = save;
    return ret;
}

Token *pp_param() {
    Token head;
    Token *cur = &head;
    int inner = 0;
    while (inner || !current_is(",") && !current_is(")")) {
        if (current_is("(")) {
            inner++;
        } else if (inner && current_is(")")) {
            inner--;
        }
        cur->next = calloc(1, sizeof(Token));
        *cur->next = *token;
        cur = cur->next;
        token = token->next;
    }
    cur->next = calloc(1, sizeof(Token));
    cur = cur->next;
    cur->str = "";
    cur->len = 0;
    return head.next;
}

// tokenを引数の内容に戻してNULLを返す
void *NULL_rewind(Token *rewind) {
    token = rewind;
    return NULL;
}

Token *pp_tiny(Token *tok);

Token *pp_macro() {
    Token *save = token;
    Token *idt = consume_identpp();
    if (!idt) return NULL;
    if (idt->forbid_expand) return NULL_rewind(save);
    Macro *mcr = find_macro(idt);
    if (!mcr) return NULL_rewind(save);

    Token **prms = mcr->params;
    if (mcr->pcnt == -1) {  // パラメタを持たないマクロ
    } else {
        // マクロの識別子の後のパラメタ部分(..)を読む
        if (!consume("(")) return NULL_rewind(save);
        if (mcr->pcnt == 0) {
            expect(")");
        } else if (mcr->pcnt == 1 && consume(")")) {
            // パラメタ1つ、何も入れないパターン
            *prms[0] = *new_tok(TK_IDENT, "", 0);
        } else {
            for (int i = 0; i < mcr->pcnt; i++) {
                *prms[i] = *pp_param();
                if (i + 1 < mcr->pcnt) expect(",");
            }
            expect(")");
        }
    }
    // マクロ展開禁止なら識別子トークンを展開する代わりにフラグを立てる
    if (mcr->forbid_expand) {
        idt->forbid_expand = true;
        return NULL_rewind(save);
    }
    mcr->forbid_expand = true;  // 自分自身を再帰展開しないようにする
    Token head;
    Token *cur = &head;
    cur->next = make_token_from_macro(mcr);
    cur->next = merge_tokens(cur->next);
    cur->next = pp_tiny(cur->next);
    mcr->forbid_expand = false;
    while (cur->next) cur = cur->next;
    cur->next = token;
    token = head.next;
    return head.next;
}

// マクロ再帰展開に使用
Token *pp_tiny(Token *tok) {
    Token *token0 = token;
    token = tok;
    Token head;
    Token *cur = &head;
    while (token) {
        Token *mtok = pp_macro();
        if (mtok) {
            token = mtok;
        } else if (token) {
            cur->next = token;
            cur = cur->next;
            token = token->next;
        }
        continue;
    }
    cur->next = token;
    token = token0;
    return head.next;
}

Token *preproc(Token *tok, char *filepath) {
    Token *token0 = token;
    token = tok;
    Token head;
    Token *cur = &head;

    while (token && !at_eof()) {
        if (consume("\n")) continue;
        if (!consume("#")) {
            if (skip) {
                newline();
            } else {
                Token *mtok = pp_macro();
                if (mtok) {
                    token = mtok;
                } else {
                    cur->next = token;
                    cur = cur->next;
                    token = token->next;
                }
            }
            continue;
        }

        if (consume("ifdef")) {
            nestif++;
            if (!skip) {
                Token *idt = consume_identpp();
                skip = !find_macro(idt);
                nestskip = nestif;
            }
        } else if (consume("ifndef")) {
            nestif++;
            if (!skip) {
                Token *idt = consume_identpp();
                skip = find_macro(idt);
                nestskip = nestif;
            }
        } else if (consume("if")) {
            nestif++;
            if (!skip) {
                if (!pp_if()) skip = true;
                nestskip = nestif;
            }
        } else if (consume("endif")) {
            if (nestskip == nestif) skip = false;
            nestif--;
        } else if (consume("else")) {
            if (!skip || nestskip == nestif) {
                skip = !skip;
                nestskip = nestif;
            }
        } else if (consume("elif")) {
            if (!skip || nestskip == nestif) {
                skip = !pp_if();
                nestskip = nestif;
            }
        } else if (skip) {
        } else if (consume("include")) {
            pp_include(filepath);
        } else if (consume("undef")) {
            Token *idt = consume_identpp();
            Macro *mcr = find_macro(idt);
            if (mcr) mcr->tok = NULL;
        } else if (consume("define")) {
            Macro *mcr = pp_define();
            mcr->next = macro;
            macro = mcr;
        }
        // 行頭 "#" のみここに来る
        newline();
    }
    cur->next = token;
    token = token0;
    return head.next;
}

// 未実装の型を処理する関数
bool read_unimplemented(char **pp, Token **curp, Token *idt) {
    // volatile , const は読み飛ばす
    if (eqtokstr(idt, "volatile") || eqtokstr(idt, "const")) {
        *pp += idt->len;
        return true;
    }
    // float, double, __builtin_va_list
    // は宣言を読むのに支障のない型に置き換える
    // (デバッグを考慮して目立つ型にする)
    if (eqtokstr(idt, "float") || eqtokstr(idt, "double") ||
        eqtokstr(idt, "__builtin_va_list")) {
        *curp = new_token(TK_TYPE, *curp, "int", 3);
        for (int i = 0; i < 10; i++)
            *curp = new_token(TK_RESERVED, *curp, "*", 1);
        *pp += idt->len;
        return true;
    }
    return false;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
    Token head;
    head.next = NULL;
    Token *tok_inc = NULL;
    Token *cur = &head;
    cur->kind = TK_RESERVED;
    cur->str = "\n";
    cur->len = 1;
    Token *word = NULL;
    while (*p) {
        if (eqtokstr(cur, "\n")) {
            word = cur;
        }
        if (word && word->next && word->next->next &&
            eqtokstr(word->next, "#") &&
            eqtokstr(word->next->next, "include")) {
            if (read_default_path(&p, &cur)) continue;
        }
        if (skip_nontoken_notLF(&p)) {
            cur->has_space = true;
            continue;
        }
        if (read_str(&p, &cur)) continue;
        if (read_char(&p, &cur)) continue;
        if (read_reserved(&p, &cur)) continue;
        if (read_num(&p, &cur)) continue;
        //先頭から変数として読める部分の長さを取得
        int idtlen = read_ident(p);
        Token *idt = new_tok(TK_IDENT, p, idtlen);
        if (read_controls(&p, &cur, idt)) continue;
        if (read_unimplemented(&p, &cur, idt)) continue;
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

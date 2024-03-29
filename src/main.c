#include "9cc.h"

char *filename;
char *filedir;
char *user_input;  // 入力プログラム
Token *token;      // 現在着目しているトークン
Node *code[CODE_LEN];
Def *dstrlits_end;

bool need_escape(int x) {
    if (x < 32) return true;
    if (x == '\'') return true;
    if (x == '\\') return true;
    return false;
}

char re_escape(int x) {
    if (x == 0) return '0';   // NULL
    if (x == 7) return 'a';   // BEL
    if (x == 8) return 'b';   // BS (Back Space)
    if (x == 9) return 't';   // HT (Horizontal Tab)
    if (x == 10) return 'n';  // LF
    if (x == 11) return 'v';  // VT (Vertical Tab)
    if (x == 12) return 'f';  // FF (Form Field)
    if (x == 13) return 'r';  // CR
    return x;
}

int main(int argc, char **argv) {
    if (argc != 2 && argc != 3) {
        error("引数の個数が正しくありません");
        return 1;
    }
    filename = argv[1];
    bool output_preproc = (argc == 3 && strncmp(argv[2], "-E", 2) == MATCH);
    init_sizes();
    init_errmsg();

    // config読み込み、トークナイズ
    char *ccdir = cpy_dirname(argv[0]);
    char *config_path = strcat(ccdir, "9cc_config.h");
    char *config = read_file(config_path);
    Token *tok_cfg = tokenize(config);

    // 引数で渡された本体ファイル読み込み、トークナイズ
    filedir = cpy_dirname(filename);
    user_input = read_file(filename);
    token = tokenize(user_input);

    // configトークン列を本体の頭につけてプリプロセッサ処理
    token = concat_tokens(tok_cfg, token);
    token = preproc(token, filename);

    // 第三引数に -E があればプリプロセッサ処理後のトークンを出力して終了
    if (output_preproc) {
        while (token && token->kind != TK_EOF) {
            if (token->kind == TK_NUM)
                printf("%d ", (int)token->val);
            else if (token->kind == TK_CHAR) {
                if (need_escape(token->val))
                    printf("'\\%c' ", (char)re_escape(token->val));
                else
                    printf("'%c' ", (char)token->val);
            } else
                printf("%.*s ", token->len, token->str);
            token = token->next;
            if (token && token->is_linehead) printf("\n");
        }
        return 0;
    }

    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    // gen global var code
    printf(".data\n");
    for (int i = 0; code[i] != NULL; i++) {
        if (code[i]->kind == ND_DEFGLOBAL) gen(code[i]);
    }
    // 文字列リテラル部のコード生成
    printf(".section .rodata\n");
    for (Def *dstrl = dstrlits_end->prev; dstrl; dstrl = dstrl->prev) {
        printf("%s:\n", dstrl->strlit->label);
        printf("  .string %.*s\n", dstrl->tok->len, dstrl->tok->str);
    }
    // 先頭の式から順にコード生成
    for (int i = 0; code[i] != NULL; i++) {
        if (code[i]->kind != ND_DEFGLOBAL) gen(code[i]);
    }

    return 0;
}

#include <errno.h>

#include "9cc.h"

char *filename;
char *user_input;  // 入力プログラム
Token *token;      // 現在着目しているトークン
Node *code[CODE_LEN];
Def *dglobals_end;
Def *dstrlits_end;

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }
    filename = argv[1];
    user_input = read_file(filename);
    init_sizes();

    // トークナイズする
    token = tokenize(user_input);

    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");

    // 文字列リテラル部のコード生成
    printf(".data\n");

    for (Def *dglb = dglobals_end->prev; dglb; dglb = dglb->prev) {
        if (!dglb->var->is_defined) continue;
        printf(".globl %.*s\n", dglb->tok->len, dglb->tok->str);
        printf("%.*s:\n", dglb->tok->len, dglb->tok->str);
        printf("  .zero %d\n", size(dglb->var->type));
    }
    printf(".section .rodata\n");
    for (Def *dstrl = dstrlits_end->prev; dstrl; dstrl = dstrl->prev) {
        printf("%s:\n", dstrl->strlit->label);
        printf("  .string %.*s\n", dstrl->tok->len, dstrl->tok->str);
    }

    // 先頭の式から順にコード生成
    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }

    return 0;
}

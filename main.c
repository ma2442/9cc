#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "9cc.h"

void init_sizes() {
    sizes[CHAR] = 1;
    sizes[BOOL] = 1;
    sizes[INT] = 4;
    sizes[PTR] = 8;
}

void init_words() {
    type_words[INT] = STR_INT;
    type_words[CHAR] = STR_CHAR;
    type_words[BOOL] = STR_BOOL;
}

// 型のサイズを計算する関数
int size(Type *typ) {
    if (typ == NULL) {
        return -1;
        // error("size : 型がありません");
    }
    if (typ->ty == ARRAY) {
        return size(typ->ptr_to) * typ->array_size;
    }
    return sizes[typ->ty];
}

// デリファレンス可能な型を判別(ARRAY, PTR)
bool can_deref(Type *typ) {
    if (typ == NULL) return false;
    if (typ->ptr_to == NULL) return false;
    return true;
}

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
    init_words();

    // トークナイズする
    token = tokenize(user_input);

    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".text\n");
    printf(".globl main\n");

    // 文字列リテラル部のコード生成
    printf(".data\n");
    Var *glb = globals_end->prev;
    while (glb) {
        printf("%.*s:\n", glb->len, glb->name);
        printf("  .zero %d\n", size(glb->type));
        glb = glb->prev;
    }
    StrLit *strl = strlits_end->prev;
    while (strl) {
        printf("%s:\n", strl->name);
        printf("  .string %.*s\n", strl->len, strl->str);
        strl = strl->prev;
    }

    // 先頭の式から順にコード生成
    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }

    return 0;
}

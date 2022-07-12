#include "9cc.h"

void init_sizes() {
    sizes[CHAR] = 1;
    sizes[INT] = 4;
    sizes[PTR] = 8;
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

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    user_input = argv[1];
    init_sizes();

    // トークナイズする
    token = tokenize(user_input);

    program();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".text\n");
    printf(".globl main\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }

    // 文字列リテラル部のコード生成
    printf(".data\n");
    while (strlits) {
        printf("%s:\n", strlits->name);
        printf("  .string %.*s\n", strlits->len, strlits->str);
        strlits = strlits->next;
    }

    return 0;
}

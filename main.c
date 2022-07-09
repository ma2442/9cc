#include "9cc.h"

void init_sizes() {
    sizes[INT] = 4;
    sizes[PTR] = 8;
}

// 型のサイズを計算する関数
size_t size(Type *typ) {
    if (typ == NULL) {
        error("size : 型がありません");
    }
    if (typ->ty == ARRAY) {
        return size(typ->ptr_to) * typ->array_size;
    }
    return sizes[typ->ty];
}

// ポインタや配列が参照する型のサイズかエラー:-1を返す
int size_deref(Node *node) {
    if (node->type == NULL || node->type->ptr_to == NULL) {
        return -1;
    }
    return sizes[node->type->ptr_to->ty];
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

    return 0;
}

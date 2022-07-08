#include "9cc.h"

void init_sizes() {
    sizes[INT] = 4;
    sizes[PTR] = 8;
    sizes[ARRAY] = sizes[PTR];
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
    printf(".globl main\n");

    // 先頭の式から順にコード生成
    for (int i = 0; code[i] != NULL; i++) {
        gen(code[i]);
    }

    return 0;
}

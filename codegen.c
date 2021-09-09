#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR) {
        error("代入の左辺値が変数ではありません");
    }
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node *node) {
    if (!node) {
        return;
    }
    char func_name[64];
    switch (node->kind) {
    case ND_RETURN:
        gen(node->lhs);
        printf("  pop rax\n");
        printf("  mov rsp, rbp\n");
        printf("  pop rbp\n");
        printf("  ret\n");
        return;
    case ND_IF_ELSE:
        gen(node->judge);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lelse%d\n", node->label_num);
        gen(node->lhs);
        printf("  jmp .Lend%d\n", node->label_num);
        printf(".Lelse%d:\n", node->label_num);
        gen(node->rhs);
        printf(".Lend%d:\n", node->label_num);
        return;
    case ND_WHILE:
        printf(".Lbegin%d:\n", node->label_num);
        gen(node->judge);
        printf("  pop rax\n");
        printf("  cmp rax, 0\n");
        printf("  je .Lend%d\n", node->label_num);
        gen(node->lhs);
        printf("  jmp .Lbegin%d\n", node->label_num);
        printf(".Lend%d:\n", node->label_num);
        return;
    case ND_FOR:
        gen(node->init);
        printf(".Lbegin%d:\n", node->label_num);
        if (node->judge) {
            gen(node->judge);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lend%d\n", node->label_num);
        }
        gen(node->lhs);
        gen(node->inc);
        printf("  jmp .Lbegin%d\n", node->label_num);
        printf(".Lend%d:\n", node->label_num);
        return;
    case ND_NUM:
        printf("  push %d\n", node->val);
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    case ND_FUNC_CALL:
        gen(node->lhs); //実引数計算
        // rsp - mod(rsp, 16) を計算してrspを16バイト境界に揃える。
        // printf("  mov rax, rsp\n");
        // printf("  mov rdi, 0x10\n");
        // printf("  cqo\n");
        // printf("  idiv rdi\n");
        // printf("  sub rsp, rdx\n");
        // rspを16バイト境界に揃える。
        printf("  xor rsp, 0x0f\n");

        // 関数呼び出し
        strncpy(func_name, node->func_name, node->func_name_len);
        func_name[node->func_name_len] = '\0';
        printf("  call %s\n", func_name);
        return;
    case ND_ACTUAL_ARG:
        gen(node->lhs); // previous arg
        gen(node->rhs); // value of this arg
        char arg_storage[][8] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
        if (arg_storage[node->arg_idx][0] != '\0') {
            printf("  pop %s\n", arg_storage[node->arg_idx]);
        }
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
        return;
    case ND_BLOCK:
        for (int i = 0; i <= 128; i++) {
            if (!node->block[i]) {
                return;
            }
            gen(node->block[i]);
        }
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    switch (node->kind) {
    case ND_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NOT_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LESS_THAN:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LESS_OR_EQUAL:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_ADD:
        printf("  add rax, rdi\n");
        break;
    case ND_SUB:
        printf("  sub rax, rdi\n");
        break;
    case ND_MUL:
        printf("  imul rax, rdi\n");
        break;
    case ND_DIV:
        printf("  cqo\n");
        printf("  idiv rdi\n");
        break;
    }

    printf("  push rax\n");
}

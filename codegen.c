#include "9cc.h"

void gen_lval(Node *node) {
    if (node->kind != ND_LVAR && node->kind != ND_GVAR) {
        error("代入の左辺値が変数ではありません");
    }
    if (node->kind == ND_GVAR) {
        printf("  lea rax, %.*s[rip]\n", node->name_len, node->name);
        printf("  push rax\n");

    } else if (node->kind == ND_LVAR) {
        printf("  lea rax, [rbp-%d]\n", node->offset);
        printf("  push rax\n");
    }
}

void gen_read(Node *node) {
    printf("  pop rax\n");
    if (size(node->type) == 1) {
        printf("  movsx rcx, BYTE PTR [rax]\n");  // CHAR 符号拡張
    } else if (size(node->type) == 4) {
        printf("  movsx rcx, DWORD PTR [rax]\n");  // INT 符号拡張
    } else {
        printf("  mov rcx, [rax]\n");
    }
    printf("  push rcx\n");
}

void gen_tochar() {
    printf("  pop rax\n");
    printf("  movzx ecx, al\n");
    printf("  push rcx\n");
}

void gen(Node *node) {
    if (!node) {
        return;
    }
    char arg_storage[][8] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int dst_size;  // 書き込み先のサイズ

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
        case ND_GVAR:
            gen_lval(node);
            if (node->type->ty == ARRAY) {
                return;
            }
            gen_read(node);
            return;
        case ND_DEFLOCAL:
            return;
        case ND_DEFGLOBAL:
            printf(".data\n");
            printf("%.*s:\n", node->name_len, node->name);
            printf("  .zero %d\n", size(node->type));
            return;
        case ND_ADDR:
            gen_lval(node->lhs);
            return;
        case ND_DEREF:
            gen(node->lhs);
            gen_read(node);
            return;
        case ND_FUNC_CALL:
            gen(node->next_arg);  //実引数計算
            // rsp - mod(rsp, 16) を計算してrspを16バイト境界に揃える。
            // 0x0fを半端な数に書き換えても正常動作するので、
            // できているかよくわからない。なくても動く。
            printf("  mov rbx, rsp\n");
            printf("  and rbx, 0x0f\n");
            printf("  sub rsp, rbx\n");
            printf("  push rbx\n");

            // 可変長引数関数に渡す浮動小数 = 0個
            printf("  mov al, 0\n");
            // 関数呼び出し
            printf("  call %.*s\n", node->name_len, node->name);

            // rspを16バイト境界揃えから元に戻す
            // できているかよくわからない。なくても動く。
            printf("  pop rbx\n");
            printf("  add rsp, rbx\n");

            // 返り値をスタックに保存
            printf("  push rax\n");
            if (size(node->type) == 1) {
                gen_tochar();
            }
            return;
        case ND_FUNC_CALL_ARG:
            gen(node->rhs);       // value of this arg
            gen(node->next_arg);  // next arg
            if (arg_storage[node->arg_idx][0] != '\0') {
                printf("  pop %s\n", arg_storage[node->arg_idx]);
            }
            return;
        case ND_FUNC_DEFINE:
            // 関数名ラベル
            printf(".text\n");
            printf("%.*s:\n", node->name_len, node->name);
            // プロローグ
            // 変数26個分の領域を確保する
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            printf("  sub rsp, 208\n");
            // 仮引数
            gen(node->next_arg);
            // 関数本文  "{" stmt* "}"
            gen(node->rhs);
            // エピローグ
            // 最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return;
        case ND_FUNC_DEFINE_ARG:
            gen_lval(node->lhs);
            printf("  push %s\n", arg_storage[node->arg_idx]);
            gen(node->next_arg);
            printf("  pop rdi\n");
            printf("  pop rax\n");
            printf("  mov [rax], rdi\n");
            // printf("  push rdi\n");
            return;
        case ND_ASSIGN:
            if (node->lhs->kind == ND_DEREF) {
                gen(node->lhs->lhs);
                dst_size = size(node->lhs->lhs->type->ptr_to);
            } else {
                gen_lval(node->lhs);
                dst_size = size(node->lhs->type);
            }
            gen(node->rhs);
            if (dst_size == 1) {
                printf("  pop rcx\n");
                printf("  pop rax\n");
                printf("  mov [rax], cl\n");
                printf("  push rcx\n");
            } else {
                printf("  pop rdi\n");
                printf("  pop rax\n");
                if (dst_size == 4)
                    printf("  mov DWORD PTR [rax], edi\n");
                else
                    printf("  mov [rax], rdi\n");
                printf("  push rdi\n");
            }
            return;
        case ND_BLOCK:
            for (int i = 0; i <= 128; i++) {
                if (!node->block[i]) {
                    return;
                }
                gen(node->block[i]);
            }
            return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    // ポインタの加減算
    if (node->kind == ND_ADD || node->kind == ND_SUB) {
        int szd_l = size_deref(node->lhs);
        int szd_r = size_deref(node->rhs);
        if (szd_l > szd_r && szd_r == -1) {
            printf("  imul rdi, %d\n", szd_l);
        } else if (szd_l < szd_r && szd_l == -1) {
            printf("  imul rax, %d\n", szd_r);
        }
    }

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

#include "9cc.h"

void gen_lval(Node* node) {
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

void gen_cmp(char* src, char* cmpval, char* cmptype, char* dest) {
    char* reg = "al";
    if (strcmp(src, "rdi") == 0)
        reg = "dil";
    else if (strcmp(src, "rcx") == 0)
        reg = "cl";
    printf("  cmp %s, %s\n", src, cmpval);
    printf("  %s %s\n", cmptype, reg);
    printf("  movzb %s, %s\n", dest, reg);
}

void gen_read(Node* node) {
    if (node->type->ty == ARRAY) {
        return;
    }
    printf("  pop rax\n");
    if (size(node->type) == 1) {
        printf("  movsx rcx, BYTE PTR [rax]\n");  // CHAR 符号拡張
    } else if (size(node->type) == 4) {
        printf("  movsx rcx, DWORD PTR [rax]\n");  // INT 符号拡張
    } else if (node->type->ty == BOOL) {
        gen_cmp("rax", "0", "setne", "rcx");
    } else {
        printf("  mov rcx, [rax]\n");
    }
    printf("  push rcx\n");
}

void gen_tochar() {
    printf("  pop rax\n");
    printf("  movsx rcx, al\n");
    printf("  push rcx\n");
}

void replicate_top() {
    printf("  pop rax\n");
    printf("  push rax\n");
    printf("  push rax\n");
}

void swap_top() {
    printf("  pop rax\n");
    printf("  pop rdi\n");
    printf("  push rax\n");
    printf("  push rdi\n");
}

void gen(Node* node) {
    if (!node) return;
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
            gen_read(node);
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
            printf("  mov rbx, rsp\n");
            printf("  and rbx, 0x0f\n");
            printf("  sub rsp, rbx\n");
            printf("  push rbx\n");

            // 可変長引数関数に渡す浮動小数 = 0個
            printf("  mov al, 0\n");
            // 関数呼び出し
            printf("  call %.*s\n", node->name_len, node->name);

            // rspを16バイト境界揃えから元に戻す
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
            // 変数分の領域を確保する
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            if (node->offset) printf("  sub rsp, %d\n", node->offset);
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
            } else {
                gen_lval(node->lhs);
            }
            if (node->assign_kind == ASN_POST_INCDEC) {
                // 後置インクリメント
                // POST INCDEC EVAL,   ASSIGN L,  ADD L
                // [addr],              addr,     addr =STACK TOP
                replicate_top();
                gen_read(node->lhs);
                swap_top();
                replicate_top();
            } else if (node->assign_kind == ASN_COMPOSITE) {
                // 前置インクリメントと複合代入
                // ASSIGN L,  ADD L
                // addr,      addr =STACK TOP
                replicate_top();
            }

            dst_size = size(node->lhs->type);
            gen(node->rhs);
            printf("  pop rcx\n");
            printf("  pop rax\n");
            if (node->lhs->type->ty == BOOL)
                gen_cmp("rcx", "0", "setne", "rcx");
            if (dst_size == 1) {
                printf("  mov [rax], cl\n");
            } else if (dst_size == 4) {
                printf("  mov DWORD PTR [rax], ecx\n");
            } else {
                printf("  mov [rax], rcx\n");
            }
            if (node->assign_kind != ASN_POST_INCDEC) printf("  push rcx\n");
            return;
        case ND_BLOCK:
            for (int i = 0; i <= 128; i++) {
                if (!node->block[i]) {
                    return;
                }
                gen(node->block[i]);
            }
            return;
        case ND_DEFGLOBAL:
        case ND_DEFLOCAL:
            if (node->lhs) gen(node->lhs);
            return;
    }

    // インクリメント･デクリメント･複合代入の場合は省略項の評価を遅らせる。
    if (node->lhs->kind == ND_DUMMY) {
        gen(node->rhs);
        swap_top();
        gen_read(node);
        printf("  pop rax\n");
        printf("  pop rdi\n");
    } else {
        gen(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
    }

    // ポインタの加減算
    if (node->kind == ND_ADD || node->kind == ND_SUB) {
        if (can_deref(node->lhs->type)) {
            printf("  imul rdi, %d\n", size(node->lhs->type->ptr_to));
        } else if (can_deref(node->rhs->type)) {
            printf("  imul rax, %d\n", size(node->rhs->type->ptr_to));
        }
    }

    switch (node->kind) {
        case ND_EQUAL:
            gen_cmp("rax", "rdi", "sete", "rax");
            break;
        case ND_NOT_EQUAL:
            gen_cmp("rax", "rdi", "setne", "rax");
            break;
        case ND_LESS_THAN:
            gen_cmp("rax", "rdi", "setl", "rax");
            break;
        case ND_LESS_OR_EQUAL:
            gen_cmp("rax", "rdi", "setle", "rax");
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
        case ND_MOD:
            printf("  cqo\n");
            printf("  idiv rdi\n");
            break;
    }

    if (node->kind == ND_MOD)
        printf("  push rdx\n");
    else
        printf("  push rax\n");
}

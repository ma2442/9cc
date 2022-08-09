#include "9cc_auto.h"

typedef enum AsmWord AsmWord;
enum AsmWord { RAX, RDI, RCX, RDX, QWORD_PTR, MOVS, MOVZ, SIZE_SEGMENT };

// レジスタや命令を扱うサイズごとに適切な名前に変換する
char* cnvword(AsmWord word, int byte) {
    switch (word) {
        case QWORD_PTR:
            if (byte == 4) return "dword ptr";
            if (byte == 2) return "word ptr";
            if (byte == 1) return "byte ptr";
            return "";
        case RAX:
            if (byte == 4) return "eax";
            if (byte == 2) return "ax";
            if (byte == 1) return "al";
            return "rax";
        case RDI:
            if (byte == 4) return "edi";
            if (byte == 2) return "di";
            if (byte == 1) return "dil";
            return "rdi";
        case RCX:
            if (byte == 4) return "ecx";
            if (byte == 2) return "cx";
            if (byte == 1) return "cl";
            return "rcx";
        case RDX:
            if (byte == 4) return "edx";
            if (byte == 2) return "dx";
            if (byte == 1) return "dl";
            return "rdx";
        case MOVS:  // 符号拡張 (byteはソースサイズ)
            if (byte == 4) return "movsxd";  // movsxd 8 byte, 4 byte
            if (byte == 2) return "movsx";   // movsx  8 byte, 2 byte
            if (byte == 1) return "movsx";   // movsx  8 byte, 1 byte
            return "mov";                    // mov    8 byte, 8 byte
        case MOVZ:  // ゼロ拡張 (byteはソースサイズ)
            if (byte == 4) return "mov";    // mov 8 byte, 4 byte (movzxも可)
            if (byte == 2) return "movzx";  // movzx 8 byte, 2 byte
            if (byte == 1) return "movzx";  // movzx 8 byte, 1 byte
            return "mov";                   // mov 8 byte, 8 byte
        case SIZE_SEGMENT:
            if (byte == 4) return ".long";
            if (byte == 2) return ".value";
            if (byte == 1) return ".byte";
            return ".qword";
    }
}

char* just(AsmWord word) { return cnvword(word, 8); }

void gen_cmp(AsmWord src, char* cmpval, char* cmptype, AsmWord dest) {
    char* src1byte = cnvword(src, 1);
    printf("  cmp %s, %s\n", cnvword(src, 8), cmpval);
    printf("  %s %s\n", cmptype, src1byte);
    printf("  movzx %s, %s\n", cnvword(dest, 8), src1byte);
}

void expand_size(AsmWord reg, Type* typ) {
    if (can_deref(typ)) return;
    int sz = size(typ);
    if (sz == 8 || sz == -1) return;
    if (typ->ty == BOOL) {
        gen_cmp(reg, "0", "setne", reg);
        return;
    }
    AsmWord mov = MOVZ;
    if (is_signed(typ)) mov = MOVS;
    if (sz == 4 && mov == MOVZ)
        printf("  mov %s, %s\n", cnvword(reg, 4), cnvword(reg, 4));
    else
        printf("  %s %s, %s\n", cnvword(mov, sz), just(reg), cnvword(reg, sz));
}

void gen_lval(Node* node) {
    switch (node->kind) {
        case ND_LVAR:  // local var
            printf("  lea rax, [rbp-%d]\n", node->def->var->offset);
            break;
        case ND_GVAR:
            if (node->def && node->def->kind == DK_STRLIT)  // str literal
                printf("  lea rax, %s[rip]\n", node->def->strlit->label);
            else  // gloval var
                printf("  lea rax, %.*s[rip]\n", node->def->tok->len,
                       node->def->tok->str);
            break;
        case ND_MEMBER:  // struct member
            gen_lval(node->lhs);
            printf("  pop rax\n");
            printf("  add rax, %d\n", node->def->var->offset);
            break;
        case ND_DEREF:  // *p = ..
            gen(node->lhs);
            return;
        case ND_CAST:
            gen_lval(node->lhs);
            printf("  pop rax\n");
            expand_size(RAX, node->type);
            printf("  push rax\n");
            return;
        default:
            error("代入の左辺値が変数ではありません");
            return;
    }
    printf("  push rax\n");
}

void gen_load(Type* typ) {
    if (typ->ty == ARRAY || typ->ty == STRUCT) return;
    printf("  pop rax\n");
    int sz = size(typ);
    AsmWord mov = MOVZ;
    if (is_signed(typ)) mov = MOVS;
    if (sz == 4 && mov == MOVZ)
        printf("  mov eax, dword ptr[rax]\n");
    else
        printf("  %s rax, %s[rax]\n", cnvword(mov, sz), cnvword(QWORD_PTR, sz));
    printf("  push rax\n");
}

void gen_store(Type* type) {
    printf("  pop rcx\n");
    printf("  pop rax\n");
    int dst_size = size(type);  // 書き込み先のサイズ
    if (type->ty == STRUCT) {
        for (int i = 0; i < dst_size; i += type->dstc->stc->align) {
            printf("  mov rdi, [rcx+%d]\n", i);
            // 8バイト境界から8バイト先までコピーの必要があるなら
            // QWORD コピーが望ましい
            if (i % 8 == 0 && i + 8 <= dst_size) {
                printf("  mov [rax+%d], rdi\n", i);
                continue;
            }
            // QWORD未満のコピーはアラインサイズごとにする
            printf("  mov %s[rax+%d], %s\n",
                   cnvword(QWORD_PTR, type->dstc->stc->align), i,
                   cnvword(RDI, type->dstc->stc->align));
        }
        return;
    }

    if (type->ty == BOOL) gen_cmp(RCX, "0", "setne", RCX);
    printf("  mov %s[rax], %s\n", cnvword(QWORD_PTR, dst_size),
           cnvword(RCX, dst_size));
}

void gen_tochar() {
    printf("  pop rax\n");
    printf("  movsx rax, al\n");
    printf("  push rax\n");
}

void copy_top() {
    printf("  pop rax\n");
    printf("  push rax\n");
    printf("  push rax\n");
}

void swap_top() {
    printf("  pop rax\n");
    printf("  pop rcx\n");
    printf("  push rax\n");
    printf("  push rcx\n");
}

void loop_judge_result(int num) {
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", num);
}

bool gen_ctrl(Node* node) {
    switch (node->kind) {
        case ND_RETURN:
            printf("# return\n");
            gen(node->lhs);
            if (node->lhs) printf("  pop rax\n");
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            printf("# return end\n");
            return true;
        case ND_IF_ELSE:
            printf("# if else\n");
            gen(node->judge);
            printf("  pop rax\n");
            printf("  cmp rax, 0\n");
            printf("  je .Lelse%d\n", node->label_num);
            gen(node->lhs);
            printf("  jmp .Lend%d\n", node->label_num);
            printf(".Lelse%d:\n", node->label_num);
            gen(node->rhs);
            printf(".Lend%d:\n", node->label_num);
            printf("# if else end\n");
            return true;
        case ND_COND_EMPTY:
            printf("# cond empty\n");
            printf("  push rax\n");
            printf("# cond empty end\n");
            return true;
        case ND_SWITCH:
            printf("# switch\n");
            gen(node->judge);
            printf("  pop rax\n");
            for (int i = 0; i < node->case_cnt; i++) {
                printf("  cmp rax, %lld\n", node->cases[i]->val);
                printf("  je .L%dcase%d\n", node->label_num,
                       node->cases[i]->label_num);
            }
            if (node->exists_default)
                printf("  jmp .Ldefault%d\n", node->label_num);
            else
                printf("  jmp .Lend%d\n", node->label_num);
            gen(node->lhs);
            printf(".Lend%d:\n", node->label_num);
            printf("# switch end\n");
            return true;
        case ND_CASE:
            printf(".L%dcase%d:\n", node->sw_num, node->label_num);
            gen(node->lhs);
            return true;
        case ND_DEFAULT:
            printf(".Ldefault%d:\n", node->sw_num);
            gen(node->lhs);
            return true;
        case ND_GOTO:
            if (node->label->str[0] != '.')
                printf("  jmp .L%d.%.*s\n", node->fn_num, node->label->len,
                       node->label->str);
            else
                printf("  jmp %.*s\n", node->label->len, node->label->str);
            return true;
        case ND_LABEL:
            printf(".L%d.%.*s:\n", node->fn_num, node->label->len,
                   node->label->str);
            gen(node->lhs);
            return true;
        case ND_DO:
            printf("# do while\n");
            printf(".Lbegin%d:\n", node->label_num);
            gen(node->lhs);
            printf(".Lcontinue%d:\n", node->label_num);
            gen(node->judge);
            loop_judge_result(node->label_num);
            printf("  jmp .Lbegin%d\n", node->label_num);
            printf(".Lend%d:\n", node->label_num);
            printf("# do while end\n");
            return true;
        case ND_WHILE:
            printf("# while\n");
            printf(".Lbegin%d:\n", node->label_num);
            printf(".Lcontinue%d:\n", node->label_num);
            gen(node->judge);
            loop_judge_result(node->label_num);
            gen(node->lhs);
            printf("  jmp .Lbegin%d\n", node->label_num);
            printf(".Lend%d:\n", node->label_num);
            printf("# while end\n");
            return true;
        case ND_FOR:
            printf("# for\n");
            gen(node->init);
            printf(".Lbegin%d:\n", node->label_num);
            if (node->judge) {
                gen(node->judge);
                loop_judge_result(node->label_num);
            }
            gen(node->lhs);
            printf(".Lcontinue%d:\n", node->label_num);
            gen(node->inc);
            printf("  jmp .Lbegin%d\n", node->label_num);
            printf(".Lend%d:\n", node->label_num);
            printf("# for end\n");
            return true;
        case ND_BLOCK:
            printf("# block\n");
            for (int i = 0; i <= BLOCK_LEN; i++) {
                if (!node->block[i]) return true;
                gen(node->block[i]);
            }
            printf("# block end\n");
            return true;
        default:
            return false;
    }
}

bool gen_func(Node* node) {
    char arg_storage[][8] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    switch (node->kind) {
        case ND_FUNC_CALL:
            printf("# func call %.*s\n", node->def->tok->len,
                   node->def->tok->str);
            gen(node->next_arg);  //実引数計算
            // rsp - mod(rsp, 16) を計算してrspを16バイト境界に揃える。
            printf("  mov rax, rsp\n");
            printf("  and rax, 0x0f\n");
            printf("  sub rsp, rax\n");
            printf("  push rax\n");

            // 可変長引数関数に渡す浮動小数 = 0個
            printf("  mov al, 0\n");
            // 関数呼び出し
            printf("  call %.*s\n", node->def->tok->len, node->def->tok->str);

            // rspを16バイト境界揃えから元に戻す
            printf("  pop rcx\n");
            printf("  add rsp, rcx\n");

            // 返り値をスタックに保存
            if (node->type->ty != VOID) {
                expand_size(RAX, node->type);
                printf("  push rax\n");
            }
            printf("# func call %.*s end\n", node->def->tok->len,
                   node->def->tok->str);
            return true;
        case ND_FUNC_CALL_ARG:
            printf("# func call arg %d\n", node->arg_idx);
            gen(node->rhs);       // value of this arg
            gen(node->next_arg);  // next arg
            if (arg_storage[node->arg_idx][0] != '\0') {
                printf("  pop %s\n", arg_storage[node->arg_idx]);
            }
            printf("# func call arg %d end\n", node->arg_idx);
            return true;
        case ND_FUNC_DEFINE:
            // 関数名ラベル
            printf(".text\n");
            printf(".globl %.*s\n", node->def->tok->len, node->def->tok->str);
            printf("%.*s:\n", node->def->tok->len, node->def->tok->str);
            // プロローグ
            // 変数分の領域を確保する
            printf("  push rbp\n");
            printf("  mov rbp, rsp\n");
            if (node->def->fn->stack_size > 0)
                printf("  sub rsp, %d\n", node->def->fn->stack_size);
            // 仮引数
            gen(node->next_arg);
            // 関数本文  "{" stmt* "}"
            gen(node->rhs);
            // エピローグ
            // 最後の式の結果がRAXに残っているのでそれが返り値になる
            printf("  mov rsp, rbp\n");
            printf("  pop rbp\n");
            printf("  ret\n");
            return true;
        case ND_FUNC_DEFINE_ARG:
            printf("# func def arg %d\n", node->arg_idx);
            gen_lval(node->lhs);
            printf("  push %s\n", arg_storage[node->arg_idx]);
            gen(node->next_arg);
            gen_store(node->lhs->type);
            printf("# func def arg %d end\n", node->arg_idx);
            // printf("  push rcx\n");
            return true;
        default:
            return false;
    }
}

bool gen_assign(Node* node) {
    switch (node->kind) {
        case ND_ASSIGN:
        case ND_ASSIGN_COMPOSITE:
            // 前置インクリメントと複合代入  ND_ASSIGN_COMPOSITE
            // ASSIGN L,  ADD L
            // addr,      addr =STACK TOP
            printf("# assign\n");
            gen_lval(node->lhs);
            if (node->kind == ND_ASSIGN_COMPOSITE) copy_top();
            gen(node->rhs);
            gen_store(node->lhs->type);
            printf("  push rcx\n");
            printf("# assign end\n");
            return true;
        case ND_ASSIGN_POST_INCDEC:
            // 後置インクリメント
            // POST INCDEC EVAL,   ASSIGN L,  ADD L
            // [addr],              addr,     addr =STACK TOP
            printf("# assign post incdec\n");
            gen_lval(node->lhs);
            copy_top();
            gen_load(node->lhs->type);
            swap_top();
            copy_top();
            gen(node->rhs);
            gen_store(node->lhs->type);
            printf("# assign post incdec end\n");
            return true;
        case ND_DEFGLOBAL:
            printf(".globl %.*s\n", node->def->tok->len, node->def->tok->str);
            printf("%.*s:\n", node->def->tok->len, node->def->tok->str);
            if (node->val)
                printf("  %s %lld\n",
                       cnvword(SIZE_SEGMENT, size(node->def->var->type)), node->val);
            else
                printf("  .zero %d\n", size(node->def->var->type));
            return true;
        case ND_DEFLOCAL:
            if (node->lhs) gen(node->lhs);
            return true;
        default:
            return false;
    }
}

bool gen_unary(Node* node) {
    switch (node->kind) {
        case ND_LVAR:
        case ND_GVAR:
        case ND_MEMBER:
            gen_lval(node);
            gen_load(node->type);
            return true;
        case ND_NUM:
            if (node->type->ty == LL || node->type->ty == ULL) {
                printf("  movabs rax, %lld\n", node->val);
                printf("  push rax\n");
            } else {
                printf("  push %lld\n", node->val);
            }
            return true;
        case ND_ADDR:
            printf("# address\n");
            gen_lval(node->lhs);
            return true;
            printf("# address end\n");
        case ND_DEREF:
            printf("# dereference\n");
            gen(node->lhs);
            gen_load(node->type);
            printf("# dereference end\n");
            return true;
        case ND_BIT_NOT:
            printf("# bit not\n");
            gen(node->lhs);
            printf("  pop rax\n");
            printf("  not rax\n");
            printf("  push rax\n");
            printf("# bit not end\n");
            return true;
        case ND_CAST:
            printf("# cast\n");
            gen(node->lhs);
            printf("  pop rax\n");
            expand_size(RAX, node->type);
            printf("  push rax\n");
            printf("# cast end\n");
            return true;
        default:
            return false;
    }
}

// ポインタ加減算時のサイズ調整
void gen_addsub_sizing(Type* l, Type* r) {
    if (can_deref(l)) {
        printf("  imul rcx, %d\n", size(l->ptr_to));
    } else if (can_deref(r)) {
        printf("  imul rax, %d\n", size(r->ptr_to));
    }
}

void gen(Node* node) {
    if (!node || node->kind == ND_NO_EVAL) return;
    if (gen_ctrl(node)) return;
    if (gen_assign(node)) return;
    if (gen_func(node)) return;
    if (gen_unary(node)) return;
    printf("# calc\n");
    // インクリメント･デクリメント･複合代入の場合は省略項の評価を遅らせる。
    if (node->lhs->kind == ND_OMITTED_TERM) {
        gen(node->rhs);
        swap_top();
        gen_load(node->type);
        swap_top();
    } else {
        gen(node->lhs);
        gen(node->rhs);
    }
    printf("  pop rcx\n");
    printf("  pop rax\n");

    // ここまででlhs, rhsともに自分の型に従い符号orゼロを8バイト伸長済
    // よって intまでの promote integer は必要なし
    // 以下の暗黙の型変換からの拡張は、符号の有無が逆転して
    // 新たに上位ビットを符号･ゼロ拡張しなければならない場合の処理
    Type* impt = implicit_type(node->lhs->type, node->rhs->type);
    if (impt && impt->ty != node->lhs->type->ty) expand_size(RAX, impt);
    if (impt && impt->ty != node->rhs->type->ty) expand_size(RCX, impt);

    switch (node->kind) {
        case ND_BIT_OR:
            printf("  or rax, rcx\n");
            break;
        case ND_BIT_XOR:
            printf("  xor rax, rcx\n");
            break;
        case ND_BIT_AND:
            printf("  and rax, rcx\n");
            break;
        case ND_EQUAL:
            printf("# calc ==\n");
            gen_cmp(RAX, "rcx", "sete", RAX);
            printf("# calc == end\n");
            break;
        case ND_NOT_EQUAL:
            printf("# calc !=\n");
            gen_cmp(RAX, "rcx", "setne", RAX);
            printf("# calc != end\n");
            break;
        case ND_LESS_THAN:
            printf("# calc <\n");
            gen_cmp(RAX, "rcx", "setl", RAX);
            printf("# calc < end\n");
            break;
        case ND_LESS_OR_EQUAL:
            printf("# calc <=\n");
            gen_cmp(RAX, "rcx", "setle", RAX);
            printf("# calc <= end\n");
            break;
        case ND_BIT_SHIFT_L:
            printf("  shl rax, cl\n");
            break;
        case ND_BIT_SHIFT_R:
            if (is_signed(node->lhs->type) && is_signed(node->rhs->type))
                printf("  sar rax, cl\n");  // signed: 符号ビットで埋める
            else
                printf("  shr rax, cl\n");  // ゼロで埋める
            break;
            printf("# calc >> \n");
        case ND_ADD:
            gen_addsub_sizing(node->lhs->type, node->rhs->type);
            printf("  add rax, rcx\n");
            break;
        case ND_SUB:
            gen_addsub_sizing(node->lhs->type, node->rhs->type);
            printf("  sub rax, rcx\n");
            break;
        case ND_MUL:
            printf("  imul rax, rcx\n");
            break;
        case ND_DIV:
        case ND_MOD:
            printf("  cqo\n");
            printf("  idiv rcx\n");
            break;
        default:
            return;
    }
    AsmWord ans = RAX;
    if (node->kind == ND_MOD) ans = RDX;
    expand_size(ans, node->type);
    printf("  push %s\n", just(ans));
    printf("# calc end\n");
}

#include "9cc.h"
int jmp_label_cnt = 0;
int str_label_cnt = 0;
int breaknest = -1;
int continest = -1;
int swnest = -1;
int fncnt = -1;
int nest = -1;

// スコープに入る
void scope_in() {
    nest++;
    def[nest] = calloc(1, sizeof(Defs));
    def[nest]->funcs = calloc_def(DK_FUNC);
    def[nest]->vars = calloc_def(DK_VAR);
    def[nest]->vars_last = def[nest]->vars;
    def[nest]->structs = calloc_def(DK_STRUCT);
}

// スコープから出る
void scope_out() {
    if (nest > 0 && def[nest]->vars_last) {
        // 関数内ネストの変数はローカル変数に載せて行く
        def[nest]->vars_last->next = fnc->fn->vars;
        fnc->fn->vars = def[nest]->vars;
    }
    free(def[nest]);
    def[nest] = NULL;
    nest--;
}

// 構造体メンバのスコープに入る
void member_in() { scope_in(); }

// 構造体メンバのスコープから出る
void member_out() {
    free(def[nest]);
    def[nest] = NULL;
    nest--;
}

// for, while, do-while スコープに入る際のbreak, continueラベル同期処理
void loop_in() {
    breaknest++;
    continest++;
    breaklcnt[breaknest] = jmp_label_cnt;
    contilcnt[continest] = jmp_label_cnt;
    jmp_label_cnt++;
}

// for, while, do-while スコープから出る際のbreak, continueラベル同期処理
void loop_out() {
    breaknest--;
    continest--;
}

// switch スコープに入る際ののbreakラベル同期処理
void switch_in() {
    breaknest++;
    breaklcnt[breaknest] = jmp_label_cnt;
    jmp_label_cnt++;
}

// switch スコープから出る際のbreakラベル同期処理
void switch_out() { breaknest--; }

#include "9cc_auto.h"

int jmp_label_cnt = 0;
int str_label_cnt = 0;
int breaknest = -1;
int continest = -1;
int swnest = -1;
int fncnt = -1;
int nest = -1;
int stcnest = -1;

// スコープに入る
void scope_in() {
    nest++;
    def[nest] = calloc(1, sizeof(Defs));
    def[nest]->dfns = calloc_def(DK_FUNC);
    def[nest]->dvars = calloc_def(DK_VAR);
    def[nest]->dvars_last = def[nest]->dvars;
    def[nest]->dstcs = calloc_def(DK_STRUCT);
    def[nest]->dtypdefs = calloc_def(DK_TYPE);
}

// スコープから出る
void scope_out() {
    if (nest > 0 && def[nest]->dvars_last) {
        // 関数内ネストの変数はローカル変数に載せて行く
        def[nest]->dvars_last->next = dfunc->fn->dvars;
        dfunc->fn->dvars = def[nest]->dvars;
    }
    free(def[nest]);
    def[nest] = NULL;
    nest--;
}

// 構造体メンバのスコープに入る
void member_in() {
    stcnest++;  // struct enum tag定義用ネスト
    scope_in();
}

// 構造体メンバのスコープから出る
void member_out() {
    free(def[nest]);
    def[nest] = NULL;
    nest--;
    stcnest--;
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

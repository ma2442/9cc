#include "9cc.h"

char *errmsg[LEN_ERRNO];

void init_errmsg() {
    errmsg[0] = "";
    errmsg[ERRNO_ERRDEFAULT] = "";
    errmsg[ERRNO_PREPROC_DEF] = "プリプロセス:定義済みの識別子です";
    errmsg[ERRNO_PREPROC_PARAMCNT] = "プリプロセス:パラメータ数が異なります";
    errmsg[ERRNO_TOKENIZE] = "トークナイズできません";
    errmsg[ERRNO_TOKENIZE_COMMENT] = "コメントが閉じられていません";
    errmsg[ERRNO_TOKENIZE_NUMSUFFIX] = "定数接尾辞ではありません";
    errmsg[ERRNO_TOKENIZE_CONST] = "整数定数ではありません";
    errmsg[ERRNO_EXPECT] = "%sではありません";
    errmsg[ERRNO_PARSE_NUM] = "数ではありません";
    errmsg[ERRNO_PARSE_TAG] = "タグがありません";
    errmsg[ERRNO_PARSE_TYPE] = "型ではありません";
    errmsg[ERRNO_PARSE_TYPEQ] = "型修飾子が不正です";
    errmsg[ERRNO_PARSE_TYPEDEF] = "typedefが不正です";
    errmsg[ERRNO_PARSE_MEMBER_HOLDER] = "メンバを持たない型です";
    errmsg[ERRNO_FIT_VAR] = "undefined valiable";
    errmsg[ERRNO_FIT_STRUCT] = "undefined struct";
    errmsg[ERRNO_FIT_UNION] = "undefined union";
    errmsg[ERRNO_FIT_ENUM] = "undefined enum";
    errmsg[ERRNO_FIT_CONSTANT] = "undefined constant";
    errmsg[ERRNO_FIT_FUNC] = "undefined function";
    errmsg[ERRNO_FIT_MEMBER] = "未定義のメンバです";
    errmsg[ERRNO_DEF_SYMBOL] = "symbol has already used";
    errmsg[ERRNO_DEF_TAG] = "tag has already used";
    errmsg[ERRNO_DECLA_VAR] = "定義済みの変数です";
    errmsg[ERRNO_DECLA_FUNC] = "定義済みの関数です";
    errmsg[ERRNO_DEF_FUNC] = "引数名がありません";
    errmsg[ERRNO_SIGNATURE] = "関数シグネチャが宣言と一致しません";
    errmsg[ERRNO_TYPE] = "宣言時の型と一致しません";
    errmsg[ERRNO_VOID] = "void型は評価できません";
    errmsg[ERRNO_RETURN] = "返却値がありません";
    errmsg[ERRNO_BREAK] = "breakがswitch, while, forの外側にあります";
    errmsg[ERRNO_CONTINUE] = "continueがswitch, while, forの外側にあります";
}

// エラーの起きた場所を報告するための関数
// 下のようなフォーマットでエラーメッセージを表示する
//
// foo.c:10: x = y + + 5;
//                   ^ 式ではありません
void error_at2(char *loc, ErrNo no, char *op) {
    fprintf(stderr, "error : ");
    fprintf(stderr, errmsg[no], op);
    fprintf(stderr, "\n");

    // locが含まれている行の開始地点と終了地点を取得
    char *line = loc;
    while (user_input < line && line[-1] != '\n') line--;

    char *end = loc;
    while (*end != '\n') end++;

    // 見つかった行が全体の何行目なのかを調べる
    int line_num = 1;
    for (char *p = user_input; p < line; p++)
        if (*p == '\n') line_num++;

    // 見つかった行を、ファイル名と行番号と一緒に表示
    int indent = fprintf(stderr, "%s:%d: ", filename, line_num);
    fprintf(stderr, "%.*s\n", (int)(end - line), line);

    // エラー箇所を"^"で指し示して、エラーメッセージを表示
    int pos = loc - line + indent;
    fprintf(stderr, "%*s", pos, "");  // pos個の空白を出力
    fprintf(stderr, "^\n");

    // va_list ap;
    // va_start(ap, fmt);
    // va_end(ap);
    // vfprintf(stderr, fmt, ap);
    exit(no);
}

void error_at(char *loc, ErrNo no, ...) { error_at2(loc, no, ""); }

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    // va_list ap;
    // va_start(ap, fmt);
    // vfprintf(stderr, fmt, ap);
    // va_end(ap);
    fprintf(stderr, fmt, NULL);
    fprintf(stderr, "\n");
    exit(ERRNO_ERRDEFAULT);
}

// エラーを報告するための関数
// printfと同じ引数を取る
void error2(char *fmt, char *p1, char *p2) {
    fprintf(stderr, fmt, p1, p2);
    fprintf(stderr, "\n");
    exit(ERRNO_ERRDEFAULT);
}

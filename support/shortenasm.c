#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);
    exit(1);
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp) error("cannot open %s: %s", path, strerror(errno));

    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // ファイル内容を読み込む
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}
typedef struct Word Word;
struct Word {
    Word *next;
    char *str;
    int len;
} noword;

typedef struct Line Line;
struct Line {
    Line *next;  // キューのように使いたいとき用 Comment
    Word head;
    char *str;
    int len;
    int no;
};

#define MATCH 0

// Wordと文字列の一致を調べる
bool same(Word *w1, Word *w2) {
    if (!w1 || !w2) return false;
    if (w1->len != w2->len) return false;
    if (strncmp(w1->str, w2->str, w1->len) != MATCH) return false;
    return true;
}

// Wordと文字列の一致を調べる
bool match(Word *w, char *str) {
    if (!w) return false;
    if (w->len != strlen(str)) return false;
    if (strncmp(w->str, str, w->len) != MATCH) return false;
    return true;
}

Word *read_word(char *str) {
    int len = 0;
    while (!isspace(str[len])) len++;
    Word *w = calloc(1, sizeof(Word));
    w->str = str;
    w->len = len;
    return w;
}

int no = 0;
// pから一行読んで 単語に切り出し行を作成する
Line *read_line(char **pp) {
    if (**pp == '\0') return NULL;
    no++;
    Line *ln = calloc(1, sizeof(Line));
    Word *wds = &ln->head;
    int len = 0;
    while ((*pp)[len] != '\n') {
        if (isspace((*pp)[len])) {
            len++;
            continue;
        }
        wds->next = read_word((*pp) + len);
        wds = wds->next;
        len += wds->len;
    }
    len++;
    ln->str = (*pp);
    ln->len = len;
    *pp += len;
    return ln;
}

bool is_push_and_pop(Word *w0, Word *w1) {
    return match(w0, "push") && match(w1, "pop") && same(w0->next, w1->next);
}

// 行を書き込み
void write_lines(Line *begin, Line *end) {
    for (Line *ln = begin; ln != end; ln = ln->next) {
        if (!ln) return;
        printf("%.*s", ln->len, ln->str);
    }
}

// その行でi番目の単語を返す(0始まり)
Word *at(Line *ln, int i) {
    if (!ln) return &noword;
    Word *w = ln->head.next;
    while (i--) {
        if (!w) break;
        w = w->next;
    }
    if (!w) w = &noword;
    return w;
}

Line *skip_comment(Line *ln) {
    while (at(ln, 0)->str[0] == '#') ln = ln->next;
    return ln;
}

// lnよりi行後ろの行を取得（コメント行を飛ばす）
Line *after(Line *ln, int i) {
    while (i--) {
        ln = skip_comment(ln);
        if (ln) ln = ln->next;
    }
    return skip_comment(ln);
}

#define LINES 5
#define COMMENT_LINES 100

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }
    char *filename = argv[1];
    char *asem = read_file(filename);

    noword.str = "";
    noword.len = 0;
    noword.next = NULL;

    Line seek;
    Line *last = &seek;
    last->next = read_line(&asem);
    if (last->next) last = last->next;

    while (seek.next) {
        while (!after(seek.next, 4)) {  // 5行作る
            if (!last) break;
            last->next = read_line(&asem);
            last = last->next;
        }

        Line *ln[4];
        Word *w[4];
        ln[0] = after(seek.next, 0);
        for (int i = 1; i < 4; i++) ln[i] = after(ln[i - 1], 1);
        for (int i = 0; i < 4; i++) w[i] = at(ln[i], 0);
        write_lines(seek.next, ln[0]);  // コメント行書き込み

        if (ln[1] && is_push_and_pop(w[0], w[1])) {  // delete 2lines
            write_lines(ln[0]->next, ln[1]);  // コメント行書き込み
            seek.next = ln[1]->next;
            continue;
        }
        // 以下のような中2行のみ有効な4行
        // push rax <-- delete
        // push rsi
        // pop rcx
        // pop rax <-- delete
        if ((ln[3] && is_push_and_pop(w[0], w[3])) && (match(w[1], "push")) &&
            (match(w[2], "pop")) && (!same(w[0]->next, w[1]->next)) &&
            (!same(w[0]->next, w[2]->next))) {
            write_lines(ln[0]->next,
                        ln[3]);  // コメント行~3行目手前まで書き込み
            seek.next = ln[3]->next;
            continue;
        }
        if (ln[0]) {
            write_lines(ln[0], ln[0]->next);
            seek.next = ln[0]->next;  // 行送り
        }
    }
    return 0;
}

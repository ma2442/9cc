#include "9cc.h"

// pathの最後に'/'が出てくるところまでをコピーして返す
char *cpy_dirname(char *path) {
    int len = 0;
    for (int i = 0; path[i] != '\0'; i++) {
        if (path[i] == '/') len = i + 1;
    }
    char *ret = calloc(255, sizeof(char));
    strncpy(ret, path, len);
    return ret;
}

// 指定されたファイルの内容を返す
char *read_file(char *path) {
    // ファイルを開く
    FILE *fp = fopen(path, "r");
    if (!fp) {
        error2("cannot open %s: %s", path, strerror(errno));
    }
    // ファイルの長さを調べる
    if (fseek(fp, 0, SEEK_END) == -1) {
        error2("%s: fseek: %s", path, strerror(errno));
    }
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) {
        error2("%s: fseek: %s", path, strerror(errno));
    }
    // ファイル内容を読み込む
    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

bool sametok(Token *tok1, Token *tok2) {
    if (!tok1 || !tok2) return false;
    if (tok1->len != tok2->len) return false;
    if (strncmp(tok1->str, tok2->str, tok1->len) != MATCH) return false;
    return true;
}

// tokenと文字列の一致を調べる
bool eqtokstr(Token *tok, char *str) {
    if (!tok) return false;
    if (tok->len != strlen(str)) return false;
    if (strncmp(tok->str, str, tok->len) != MATCH) return false;
    return true;
}

// トークン列t1の後ろにt2を連結する
// t1にEOFが含まれていればそれを消してt2につなげる
Token *concat_tokens(Token *t1, Token *t2) {
    Token head;
    Token *cur = &head;
    cur->next = t1;
    while (cur->next && cur->next->kind != TK_EOF) {
        cur = cur->next;
    }
    cur->next = t2;
    return head.next;
}



// tokenを引数の内容に戻してNULLを返す
void *NULL_rewind(Token *rewind) {
    token = rewind;
    return NULL;
}
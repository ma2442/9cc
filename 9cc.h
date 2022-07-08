#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HEADER_H
#define HEADER_H
// トークンの種類
typedef enum {
    TK_RESERVED,  // 記号
    TK_IDENT,     // 識別子
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
    TK_SIZEOF,    // sizeof
    TK_RETURN,    // return
    TK_CTRL,      // if, else, while, for等 制御構文を表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind;  // トークンの型
    Token *next;     // 次の入力トークン
    int val;         // kindがTK_NUMの場合、その数値
    char *str;       // トークン文字列
    int len;         // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
    ND_EQUAL,            // ==
    ND_NOT_EQUAL,        // !=
    ND_LESS_THAN,        // <
    ND_LESS_OR_EQUAL,    // <=
    ND_ADD,              // +
    ND_SUB,              // -
    ND_MUL,              // *
    ND_DIV,              // /
    ND_ASSIGN,           // =
    ND_DEFLOCAL,         // local valiable definition
    ND_LVAR,             // local valiable
    ND_ADDR,             // pointer &
    ND_DEREF,            // pointer *
    ND_FUNC_CALL,        // 関数呼び出し
    ND_FUNC_CALL_ARG,    // 関数の実引数
    ND_FUNC_DEFINE,      // 関数定義
    ND_FUNC_DEFINE_ARG,  // 関数の仮引数
    ND_NUM,              // 整数
    ND_RETURN,           // return
    ND_IF_ELSE,          // if (judge) lhs else rhs
    ND_WHILE,            // while (judge) lhs
    ND_FOR,              // for (init; judge; inc) lhs
    ND_BLOCK,            // block { }
} NodeKind;

typedef struct Type Type;
typedef enum { INT, PTR, ARRAY, LEN_TYPE_KIND } TypeKind;
// 型
struct Type {
    TypeKind ty;
    struct Type *ptr_to;
    size_t array_size;
};

typedef struct Node Node;

// 抽象構文木のノード
struct Node {
    NodeKind kind;      // ノードの種類
    Node *lhs;          // 左辺, またはif,while,for等の内部statement
    Node *rhs;          // 右辺, またはelseの内部statement
    int val;            // kindがND_NUMの場合のみ使う
    int offset;         // kindがND_LVARの場合のみ使う
    Type *type;         // int, int*などの型情報
    int label_num;      // if,while,for等のラベル通し番号
    Node *next_arg;     // kindがND_FUNC_* の場合に使う
    char *func_name;    // kindがND_FUNC_* の場合に使う
    int func_name_len;  // kindがND_FUNC_* の場合に使う
    int arg_idx;        // ND_FUNC_*_ARGの場合の引数番号(0始まり)
    Node *init;         // forの初期化式(_____; ; )
    Node *judge;        // if,while,for等の条件式
    Node *inc;          // forの後処理式( ; ;____)
    Node **block;       // block {} 内のstatements
};

typedef struct LVar LVar;

// ローカル変数
struct LVar {
    LVar *next;  // 次の変数かNULL
    char *name;  // 変数の名前
    int len;     // 名前の長さ
    int offset;  // RBPからのオフセット
    Type *type;  // 型
};

typedef struct Func Func;

// 関数名と引数情報
struct Func {
    Func *next;
    char *name;
    int len;  // 名前の長さ
    LVar *args;
    Type *type;
};

int sizes[LEN_TYPE_KIND];

// 関数
Func *funcs;
// ローカル変数
LVar *locals;

#endif  // HEADER_H
extern void init_sizes();
extern int size_deref(Node *node);
extern Node *new_node();
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern Token *token;

extern char *user_input;
extern int label_cnt;
extern void error_at(char *loc, char *fmt, ...);
extern void error(char *fmt, ...);
extern bool consume(char *op);
extern Token *consume_ident();
extern void expect(char *op);
extern int expect_number();
extern bool at_eof();
extern Token *new_token(TokenKind kind, Token *cur, char *str, int len);
extern Token *tokenize(char *p);
extern Node *code[100];
extern Node *statement[100];
extern Node *primary();
extern Node *unary();
extern Node *mul();
extern Node *add();
extern Node *relational();
extern Node *equality();
extern Node *assign();
extern Node *expr();
extern Node *stmt();
extern void program();

extern void gen_lval(Node *node);
extern void gen(Node *node);

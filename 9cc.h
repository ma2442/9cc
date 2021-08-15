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
    TK_RESERVED, // 記号
    TK_NUM,      // 整数トークン
    TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind; // トークンの型
    Token *next;    // 次の入力トークン
    int val;        // kindがTK_NUMの場合、その数値
    char *str;      // トークン文字列
    int len;        // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
    ND_EQUAL,         // ==
    ND_NOT_EQUAL,     // !=
    ND_LESS_THAN,     // <
    ND_LESS_OR_EQUAL, // <=
    ND_ADD,           // +
    ND_SUB,           // -
    ND_MUL,           // *
    ND_DIV,           // /
    ND_NUM,           // 整数
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind; // ノードの型
    Node *lhs;     // 左辺
    Node *rhs;     // 右辺
    int val;       // kindがND_NUMの場合のみ使う
};
#endif //HEADER_H

extern Node *new_node();
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern Token *token;

extern char *user_input;
extern void error_at(char *loc, char *fmt, ...);
extern void error(char *fmt, ...);
extern bool consume(char *op);
extern void expect(char *op);
extern int expect_number();
extern bool at_eof();
extern Token *new_token(TokenKind kind, Token *cur, char *str, int len);
extern Token *tokenize(char *p);
extern Node *primary();
extern Node *unary();
extern Node *mul();
extern Node *add();
extern Node *relational();
extern Node *equality();
extern Node *expr();

extern void gen(Node *node);

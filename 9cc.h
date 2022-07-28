#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HEADER_H
#define HEADER_H
#define BLOCK_LEN 128
#define CODE_LEN 100
#define STMT_LEN 100
#define STR_INT "int"
#define STR_CHAR "char"
#define STR_BOOL "_Bool"
#define STR_STRUCT "struct"
#define STR_ENUM "enum"

typedef struct Token Token;
typedef struct Node Node;
typedef struct Type Type;
typedef struct Func Func;
typedef struct Struct Struct;
typedef struct StrLit StrLit;
typedef struct Enum Enum;
typedef struct EnumConst EnumConst;
typedef struct Var Var;
typedef struct Symbol Symbol;
typedef struct Def Def;

// トークンの種類
typedef enum {
    TK_RESERVED,  // 記号
    TK_TYPE,      // 型
    TK_IDENT,     // 識別子
    TK_NUM,       // 整数トークン
    TK_EOF,       // 入力の終わりを表すトークン
    TK_SIZEOF,    // sizeof
    TK_RETURN,    // return
    TK_CTRL,      // if, else, while, for等 制御構文を表すトークン
    TK_STR,       // 文字列リテラルを表すトークン
    TK_STRUCT,
    TK_ENUM
} TokenKind;

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
    ND_MOD,              // %
    ND_ASSIGN,           // =
    ND_DEFLOCAL,         // local variable definition
    ND_LVAR,             // local variable, or x++, x--
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
    ND_DEFGLOBAL,        // global variable definition
    ND_GVAR,             // global variable, or x++, x--
    ND_DUMMY,            // x++,--x,複合代入等により省略された項
    ND_MEMBER,           // 構造体メンバへのアクセス
    ND_NO_EVAL           // 評価不要なノード
} NodeKind;

typedef enum {
    CHAR,
    INT,
    BOOL,
    STRUCT,
    ENUM,
    PTR,
    ARRAY,
    LEN_TYPE_KIND
} TypeKind;

// 型
struct Type {
    TypeKind ty;
    Type *ptr_to;
    size_t array_size;
    Struct *strct;
    Enum *enm;
};

// ローカル変数
struct Var {
    Var *prev;
    Var *next;     // 次の変数かNULL
    char *name;    // 変数の名前
    int len;       // 名前の長さ
    int offset;    // RBPからのオフセット
    Type *type;    // 型
    bool islocal;  // ローカル変数かどうか
};

// 関数名と引数情報
struct Func {
    Func *next;
    char *name;
    int len;         // 名前の長さ
    Var *args;       // 引数情報
    Var *vars;       // 変数情報(引数含む)
    int stack_size;  // 変数分の確保領域
    Type *type;
};

// 文字列リテラル
struct StrLit {
    StrLit *prev;
    StrLit *next;
    char *str;   // 文字列リテラル本体
    int len;     // 本体の長さ
    char *name;  // 名前(ラベル)
};

struct Struct {
    Struct *next;
    char *name;
    int len;
    Var *mems;
    int size;
    int align;
};

struct Enum {
    Enum *next;
    char *name;
    int len;
    EnumConst *consts;
};

// 列挙体で定義された定数
struct EnumConst {
    EnumConst *next;  // 次の変数かNULL
    char *name;       // 変数の名前
    int len;          // 名前の長さ
    int val;
    Enum *enm;  // 属している列挙体
};

// シンボル
struct Symbol {
    Func *func;
    Var *var;
    EnumConst *enumconst;
};

// 定義をまとめたもの(関数 変数, enum, struct)
struct Def {
    Func *funcs;
    Var *vars;
    Var *vars_last;  // ブロック内で最初に定義された変数
    Enum *enums;
    Struct *structs;
};

// 抽象構文木のノード
struct Node {
    NodeKind kind;   // ノードの種類
    Node *lhs;       // 左辺, またはif,while,for等の内部statement
    Node *rhs;       // 右辺, またはelseの内部statement
    Var *var;        // 変数情報
    Func *func;      // 関数情報
    StrLit *strlit;  // 文字列リテラル情報
    int val;         // kindがND_NUMの場合のみ使う
    Type *type;      // int, int*などの型情報
    int label_num;   // if,while,for等のラベル通し番号
    Node *next_arg;  // kindがND_FUNC_* の場合に使う
    int arg_idx;     // ND_FUNC_*_ARGの場合の引数番号(0始まり)
    Node *init;      // forの初期化式(_____; ; )
    Node *judge;     // if,while,for等の条件式
    Node *inc;       // forの後処理式( ; ;____)
    Node **block;    // block {} 内のstatements
    enum { ASN_NORMAL, ASN_POST_INCDEC, ASN_COMPOSITE } assign_kind;
};

size_t sizes[LEN_TYPE_KIND];
char *type_words[LEN_TYPE_KIND];

// 現在定義中の関数
Func *fn;
// グローバル変数
Var *globals_end;
// 文字列リテラル
StrLit *strlits;
StrLit *strlits_end;

// def[0]: グローバル 関数, 変数, struct定義, enum定義
// def[1]: 関数直下の ローカル 変数（引数含む）, struct定義, enum定義
// (関数:NULL)。 以降スコープがネストするたびに添え字が一つ増える。
// また、structメンバの定義･アクセスにも一時的に使用される。
Def *def[100];

// 現在のネストの深さ(0:global)
int nest;

// 入力ファイル名
char *filename;

#endif  // HEADER_H
extern int size(Type *typ);
extern bool can_deref(Type *typ);
extern Node *new_node();
extern Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
extern Node *new_node_num(int val);
extern Token *token;

extern char *user_input;
extern int jmp_label_cnt;
extern void error_at(char *loc, char *fmt, ...);
extern void error(char *fmt, ...);
extern bool consume(char *op);
extern Token *consume_ident();
extern Token *consume_type();
extern void expect(char *op);
extern int expect_number();
extern bool at_eof();
extern Token *new_token(TokenKind kind, Token *cur, char *str, int len);
extern Token *tokenize(char *p);
extern Node *code[CODE_LEN];
extern Node *statement[STMT_LEN];
extern void scope_in();
extern void scope_out();
extern void member_in();
extern void member_out();
extern Node *regex();
extern Node *primary();
extern Node *unary();
extern Node *mul();
extern Node *add();
extern Node *relational();
extern Node *equality();
extern Node *bool_or();
extern Node *assign();
extern Node *expr();
extern Node *stmt();
extern Node *declaration_var(Type *typ, Token *tok);
extern void program();

extern void gen_lval(Node *node);
extern void gen(Node *node);

// type.c
extern void init_sizes();
extern void init_words();
extern int size(Type *typ);
extern bool can_deref(Type *typ);
extern int align(int x, int aln);
extern int calc_align(Type *type);
extern int set_offset(Var *var, int base);
extern void typing(Node *node);
extern Type *base_type();

// find.c
extern Var *find_var(Token *tok);
extern Func *find_func(Token *tok);
extern Struct *find_struct(Token *tok);
extern EnumConst *find_enumconst(Token *tok);
extern Enum *find_enum(Token *tok);
extern Var *fit_var(Token *tok);
extern Func *fit_func(Token *tok);
extern Struct *fit_struct(Token *tag);
extern Enum *fit_enum(Token *tag);
extern EnumConst *fit_enumconst(Token *tag);
extern Symbol *fit_symbol(Token *tok);
extern bool can_def_symbol(Token *sym);
extern bool can_def_tag(Token *sym);

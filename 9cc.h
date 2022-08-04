// #include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef HEADER_H
#define HEADER_H
#define BLOCK_LEN 128
#define CASE_LEN 128
#define CODE_LEN 100
#define STMT_LEN 100
#define DIGIT_LEN 16  // ラベル番号の最大桁数
#define NEST_MAX 128
#define STR_INT "int"
#define STR_CHAR "char"
#define STR_BOOL "_Bool"
#define STR_VOID "void"
#define STR_SIGNED "signed"
#define STR_UNSIGNED "unsigned"
#define STR_SHORT "short"
#define STR_LONG "long"

typedef struct Token Token;
typedef struct Node Node;
typedef struct Type Type;
typedef struct Func Func;
typedef struct Struct Struct;
typedef struct StrLit StrLit;
typedef struct Enum Enum;
typedef struct EnumConst EnumConst;
typedef struct Var Var;
typedef struct Defs Defs;
typedef struct Def Def;

// トークンの種類
typedef enum {
    TK_RESERVED,      // 記号
    TK_TYPE,          // 型
    TK_TYPEQ_SIGN,    // 型修飾子 signed unsigned
    TK_TYPEQ_LENGTH,  // 型修飾子 short long
    TK_IDENT,         // 識別子
    TK_NUM,           // 整数トークン
    TK_NUMSUFFIX,     // 定数接尾辞トークン ull llu u ll
    TK_CHAR,          // 文字型トークン
    TK_EOF,           // 入力の終わりを表すトークン
    TK_SIZEOF,        // sizeof
    TK_RETURN,        // return
    TK_CTRL,  // if, else, while, for等 制御構文を表すトークン
    TK_STR,   // 文字列リテラルを表すトークン
    TK_STRUCT,
    TK_ENUM
} TokenKind;

// トークン型
struct Token {
    TokenKind kind;          // トークンの型
    Token *next;             // 次の入力トークン
    unsigned long long val;  // kindがTK_NUMの場合、その数値
    char *str;               // トークン文字列
    int len;                 // トークンの長さ
};

// 抽象構文木のノードの種類
typedef enum {
    ND_NO_EVAL,  // 評価不要なノード
    ND_RETURN,   // return
    ND_IF_ELSE,  // if (judge) lhs else rhs
    ND_COND_EMPTY,  // a?(empty):b 三項演算子コロン前が空のとき、aを返す
    ND_SWITCH,              // switch (judge) lhs
    ND_CASE,                // case _:
    ND_DEFAULT,             // default:
    ND_GOTO,                // jmp
    ND_LABEL,               // label(goto):
    ND_WHILE,               // while (judge) lhs
    ND_DO,                  // do lhs while (judge)
    ND_FOR,                 // for (init; judge; inc) lhs
    ND_BLOCK,               // block { }
    ND_FUNC_CALL,           // 関数呼び出し
    ND_FUNC_CALL_ARG,       // 関数の実引数
    ND_FUNC_DEFINE,         // 関数定義
    ND_FUNC_DEFINE_ARG,     // 関数の仮引数
    ND_ASSIGN,              // =
    ND_ASSIGN_COMPOSITE,    // += -= *= /= %=
    ND_ASSIGN_POST_INCDEC,  // x=x+1 x=x-1 of x++ x--
    ND_DEFLOCAL,            // local variable definition
    ND_DEFGLOBAL,           // global variable definition
    ND_ADDR,                // pointer &
    ND_DEREF,               // pointer *
    ND_BIT_NOT,             // ~
    ND_CAST,                // (int *) など
    ND_LVAR,                // local variable, evaluation of x++ x--
    ND_GVAR,                // global variable, or x++, x--
    ND_MEMBER,              // 構造体メンバへのアクセス
    ND_NUM,                 // 整数
    ND_OMITTED_TERM,   // x++,--x,複合代入等により省略された項
    ND_BIT_OR,         // |
    ND_BIT_XOR,        // ^
    ND_BIT_AND,        // &
    ND_EQUAL,          // ==
    ND_NOT_EQUAL,      // !=
    ND_LESS_THAN,      // <
    ND_LESS_OR_EQUAL,  // <=
    ND_BIT_SHIFT_L,    // <<
    ND_BIT_SHIFT_R,    // >>
    ND_ADD,            // +
    ND_SUB,            // -
    ND_MUL,            // *
    ND_DIV,            // /
    ND_MOD,            // %
} NodeKind;

typedef enum {
    UCHAR,
    USHORT,
    UINT,
    ULL,
    STRUCT,
    ENUM,
    PTR,
    ARRAY,
    VOID,
    BOOL,
    SIGNED = 16,
    CHAR = UCHAR | SIGNED,
    SHORT = USHORT | SIGNED,
    INT = UINT | SIGNED,
    LL = ULL | SIGNED,

    LEN_TYPE_KIND
} TypeKind;

// 型
struct Type {
    TypeKind ty;
    Type *ptr_to;
    size_t array_size;
    Def *strct;
    Def *enm;
};

// ローカル変数
struct Var {
    int offset;    // RBPからのオフセット
    Type *type;    // 型
    bool islocal;  // ローカル変数かどうか
};

// 関数名と引数情報
struct Func {
    Def *args;       // 引数情報
    Def *vars;       // 変数情報(引数含む)
    int stack_size;  // 変数分の確保領域
    Type *type;
};

// 文字列リテラル
struct StrLit {
    char *label;  // 名前(ラベル)
};

struct Struct {
    Def *mems;
    int size;
    int align;
};

struct Enum {
    Def *consts;
};

// 列挙子
struct EnumConst {
    int val;
    Def *enm;  // 属している列挙体
};

// 定義をまとめたもの(関数 変数, enum, struct)
struct Defs {
    Def *funcs;
    Def *vars;
    Def *vars_last;  // ブロック内で最初に定義された変数
    Def *enums;
    Def *structs;
};

// 定義の種類
typedef enum {
    DK_VAR,
    DK_FUNC,
    DK_STRUCT,
    DK_ENUM,
    DK_ENUMCONST,
    DK_STRLIT,
    DK_LEN
} DefKind;

// 変数 関数 構造体 列挙体 列挙子 文字列リテラル の定義情報
struct Def {
    Def *next;  // 次の変数かNULL
    Def *prev;
    Token *tok;  // 名前
    DefKind kind;
    union {
        Var *var;
        Func *fn;
        Struct *stc;
        Enum *enm;
        EnumConst *cst;
        StrLit *strlit;
    };
};

// 抽象構文木のノード
struct Node {
    NodeKind kind;   // ノードの種類
    Node *lhs;       // 左辺, またはif,while,for等の内部statement
    Node *rhs;       // 右辺, またはelseの内部statement
    Node *init;      // forの初期化式(_____; ; )
    Node *judge;     // if,while,for等の条件式
    Node *inc;       // forの後処理式( ; ;____)
    Node *next_arg;  // kindがND_FUNC_* の場合に使う
    Node **block;    // block {} 内のstatements
    Node **cases;    // switch{} 内のcases
    Def *def;        // 変数情報 関数情報 文字列リテラル情報
    Token *label;    // jump label
    Type *type;      // int, int*などの型情報
    unsigned long long val;  // 評価値が定数になる場合の数値
    int arg_idx;          // ND_FUNC_*_ARGの場合の引数番号(0始まり)
    bool exists_default;  // switch内にdefaultがあるか
    int label_num;  // if,while,do-while,for,switchのラベル通し番号
                    // switch内caseラベル番号
    int fn_num;     // gotoラベルのprefix 関数番号
    int sw_num;     // caseの親switchラベル番号
    int case_cnt;   // switch内のcaseの数
};

size_t sizes[LEN_TYPE_KIND];
char *type_words[LEN_TYPE_KIND];

char *filename;    // 入力ファイル名
char *user_input;  // 入力ソース
Token *token;
Node *code[CODE_LEN];
Node *statement[STMT_LEN];

// def[0]: グローバル 関数, 変数, struct定義, enum定義
// def[1]: 関数直下の ローカル 変数（引数含む）, struct定義, enum定義
// (関数:NULL)。 以降スコープがネストするたびに添え字が一つ増える。
// また、structメンバの定義･アクセスにも一時的に使用される。
Defs *def[NEST_MAX];

Def *fnc;                 // 現在定義中の関数
Def *globals_end;         // グローバル変数(出現順)
Def *strlits;             // 文字列リテラル(出現逆順)
Def *strlits_end;         // 文字列リテラル(出現順)
int jmp_label_cnt;        // jmpラベル通し番号
int str_label_cnt;        // 文字列リテラル ラベル通し番号
int nest;                 // 現在のネストの深さ(0:global)
int breaklcnt[NEST_MAX];  // break対象ラベル番号
int breaknest;            // breakネスト数
int contilcnt[NEST_MAX];  // continue対象ラベル番号
int continest;            // continueネスト数
Node *sw[NEST_MAX];       // switch対象ノード
int swnest;               // switchネスト数
int fncnt;                // 関数通し番号(goto label 用)

#endif  // HEADER_H
int size(Type *typ);
bool can_deref(Type *typ);
Def *calloc_def(DefKind kind);
Node *new_node();
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(unsigned long long val);

// error.c
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

// parse.c
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(char *p);
Node *regex();
Node *primary();
Node *unary();
Node *mul();
Node *add();
Node *relational();
Node *equality();
Node *bool_or();
Node *condition();
Node *assign();
Node *expr();
Node *stmt();
Node *labeled();
Node *declaration_var(Type *typ, Token *tok);
void program();

// codegen.c
void gen(Node *node);

// type.c
void init_sizes();
int val(Node *node);  // 定数計算
int size(Type *typ);
bool can_deref(Type *typ);
int align(int x, int aln);
int calc_align(Type *type);
int set_offset(Var *var, int base);
bool is_signed(Type *typ);
void typing(Node *node);
Type *base_type();
void voidcheck(Type *typ, char *pos);
Type *implicit_type(Type *lt, Type *rt);
Type *promote_integer(Type *typ);
int priority(Type *typ);

// consume.c
bool consume(char *op);
Token *consume_numeric();
Token *consume_str();
Token *consume_typeq();
Token *consume_type();
Token *consume_ident();
Token *consume_numsuffix();
int consume_incdec();
int consume_compo_assign();
void expect(char *op);
int expect_numeric();
bool at_eof();

// find.c
Def *find_def(Token *tok, DefKind kind);
Def *find_enumconst(Token *tok);
Def *fit_def(Token *tok, DefKind kind);
bool can_def_symbol(Token *sym);
bool can_def_tag(Token *sym);

// scope.c
void scope_in();
void scope_out();
void member_in();
void member_out();
void loop_in();
void loop_out();
void switch_in();
void switch_out();

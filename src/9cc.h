#ifndef HEADER_H
#define HEADER_H
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #define __DEBUG__READ__FILE__
#define BLOCK_LEN 1280
#define CASE_LEN 128
#define CODE_LEN 1280
#define STMT_LEN 1280
#define DIGIT_LEN 16  // ラベル番号の最大桁数
#define NEST_MAX 128
#define MATCH 0
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

typedef enum {
    ERRNO_ERRDEFAULT = 1,
    ERRNO_PREPROC_DEF,
    ERRNO_PREPROC_PARAMCNT,
    ERRNO_TOKENIZE,
    ERRNO_TOKENIZE_NUMSUFFIX,
    ERRNO_TOKENIZE_COMMENT,
    ERRNO_TOKENIZE_CONST,
    ERRNO_EXPECT,
    ERRNO_PARSE_NUM,
    ERRNO_PARSE_TAG,
    ERRNO_PARSE_TYPEQ,
    ERRNO_PARSE_TYPE,
    ERRNO_PARSE_TYPEDEF,
    ERRNO_PARSE_MEMBER_HOLDER,
    ERRNO_FIT_VAR,
    ERRNO_FIT_STRUCT,
    ERRNO_FIT_UNION,
    ERRNO_FIT_ENUM,
    ERRNO_FIT_CONSTANT,
    ERRNO_FIT_FUNC,
    ERRNO_FIT_MEMBER,
    ERRNO_DEF_SYMBOL,
    ERRNO_DEF_TAG,
    ERRNO_DECLA_VAR,
    ERRNO_DECLA_FUNC,
    ERRNO_DEF_FUNC,
    ERRNO_SIGNATURE,
    ERRNO_TYPE,
    ERRNO_VOID,
    ERRNO_RETURN,
    ERRNO_BREAK,
    ERRNO_CONTINUE,
    LEN_ERRNO
} ErrNo;

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
    TK_DEFAULT_PATH,  // <..> include path
    TK_STRUCT,
    TK_UNION,
    TK_ENUM
} TokenKind;

// トークン型
struct Token {
    TokenKind kind;          // トークンの型
    Token *next;             // 次の入力トークン
    unsigned long long val;  // kindがTK_NUMの場合、その数値
    char *str;               // トークン文字列
    int len;                 // トークンの長さ
    bool has_space;          // うしろにスペースがあるか
    bool is_linehead;        // 行頭のトークン
    bool need_merge;  // 後ろにトークン連結演算子 ## があったか
    bool forbid_expand;  //マクロ展開可能か
};

// 型の種類
typedef enum {
    VARIABLE,  // printfのような可変個数の引数
    UCHAR,
    USHORT,
    UINT,
    ULL,
    STRUCT,
    UNION = STRUCT,
    PTR,
    ARRAY,
    FUNC,
    VOID,
    BOOL,
    SIGNED = 16,
    CHAR = UCHAR | SIGNED,
    SHORT = USHORT | SIGNED,
    INT = UINT | SIGNED,
    ENUM = INT,
    LL = ULL | SIGNED,

    LEN_TYPE_KIND
} TypeKind;

// 型
struct Type {
    TypeKind ty;
    Type *ptr_to;
    int array_size;
    Def *dfn;
    Def *dstc;
    Def *denm;
};

// ローカル変数
struct Var {
    int offset;       // RBPからのオフセット
    Type *type;       // 型
    bool islocal;     // ローカル変数かどうか
    bool is_defined;  // 定義済みか、宣言のみか
    bool is_static;   // staticかどうか (グローバル変数のみ)
};

// 関数名と引数情報
struct Func {
    Def *darg0;       // 第一引数の手前
    Def *dargs;       // 引数情報
    Def *dvars;       // 変数情報(引数含む)
    int stack_size;   // 変数分の確保領域
    Type *ret;        // 返却型
    bool is_defined;  // 定義済みか、宣言のみか
    bool can_define;  // 型として見た場合に定義可能か
                      // (全ての引数に名前が設定されているか)
    bool is_static;   // staticかどうか
};

// 文字列リテラル
struct StrLit {
    char *label;  // 名前(ラベル)
};

// struct or union
struct Struct {
    Def *dmems;
    int size;
    int align;
};

struct Enum {
    Def *dconsts;
};

// 列挙子
struct EnumConst {
    int val;
    Def *denm;  // 属している列挙体
};

// 定義の種類
typedef enum {
    DK_VAR,
    DK_FUNC,
    DK_STRUCT,
    DK_UNION,
    DK_ENUM,
    DK_ENUMCONST,  // 列挙子
    DK_STRLIT,     // 文字列リテラル
    DK_TYPE,       // typedef された型
    DK_LEN
} DefKind;

// 変数 関数 構造体 列挙体 列挙子 文字列リテラル の定義情報
struct Def {
    Def *next;  // 次の変数かNULL
    Def *prev;
    Token *tok;  // 名前
    DefKind kind;
    Var *var;
    Func *fn;
    Struct *stc;
    Enum *enm;
    EnumConst *cst;
    StrLit *strlit;
    Type *defdtype;
};

// 定義をまとめたもの(関数 変数, enum, struct)
struct Defs {
    Def *dfns;
    Def *dvars;
    Def *dvars_last;  // ブロック内で最初に定義された変数
    Def *denms;
    Def *dstcs;
    Def *dunis;
    Def *dtypdefs;
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

extern char *errmsg[LEN_ERRNO];
extern int sizes[LEN_TYPE_KIND];

extern char *filename;    // 入力ファイル名
extern char *filedir;     // 入力ファイルのディレクトリ
extern char *user_input;  // 入力ソース
extern Token *token;
extern Node *code[CODE_LEN];
extern Node *statement[STMT_LEN];

// def[0]: グローバル 関数, 変数, struct定義, enum定義
// def[1]: 関数直下の ローカル 変数（引数含む）, struct定義, enum定義
// (関数:NULL)。 以降スコープがネストするたびに添え字が一つ増える。
// また、structメンバの定義･アクセスにも一時的に使用される。
extern Defs *def[NEST_MAX];

extern Def *dfunc;               // 現在定義中の関数
extern Def *dstrlits;            // 文字列リテラル(出現逆順)
extern Def *dstrlits_end;        // 文字列リテラル(出現順)
extern int jmp_label_cnt;        // jmpラベル通し番号
extern int str_label_cnt;        // 文字列リテラル ラベル通し番号
extern int nest;                 // 現在のネストの深さ(0:global)
extern int stcnest;              // 構造体のネストの深さ
extern int breaklcnt[NEST_MAX];  // break対象ラベル番号
extern int breaknest;            // breakネスト数
extern int contilcnt[NEST_MAX];  // continue対象ラベル番号
extern int continest;            // continueネスト数
extern Node *sw[NEST_MAX];       // switch対象ノード
extern int swnest;               // switchネスト数
extern int fncnt;                // 関数通し番号(goto label 用)

// util.c
char *cpy_dirname(char *path);
char *read_file(char *path);
bool sametok(Token *tok1, Token *tok2);
bool eqtokstr(Token *tok, char *str);
Token *concat_tokens(Token *t1, Token *t2);
void *NULL_rewind(Token *rewind);

char *read_file(char *path);
char *cpy_dirname(char *path);

// error.c
void init_errmsg();
void error_at2(char *loc, ErrNo no, char *op);
void error_at(char *loc, ErrNo no, ...);
void error(char *fmt, ...);
void error2(char *fmt, char *p1, char *p2);

// tokenize.c
Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize(char *p);
Token *preproc(Token *tok, char *filepath);

// parse.c
Def *calloc_def(DefKind kind);
Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(unsigned long long val);
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
Node *localtop();
Node *defvar(Type *typ, Token *tok);
void program();

// codegen.c
void gen(Node *node);

// type.c
Type *new_type(TypeKind kind);
long long val(Node *node);  // 定数式計算
void init_sizes();
int size(Type *typ);
bool eqtype(Type *typ1, Type *typ2);
bool can_deref(Type *typ);
bool can_cast(Type *to, Type *from);
bool is_signed(Type *typ);
int priority(Type *typ);
Type *promote_integer(Type *typ);
Type *implicit_type(Type *lt, Type *rt);
void typing(Node *node);
void voidcheck(Type *typ, char *pos);
int align(int x, int aln);
int calc_align(Type *type);
int set_offset(Var *var, int base);
Node *typdef();
Type *defdtype();
Type *type_full(Token **idtp);

// consume.c
Token *consume_identpp();
bool current_is(char *op);
bool consume(char *op);
Token *consume_numeric();
Token *consume_str();
Token *consume_typeq();
Token *consume_tag_without_def(TokenKind kind);
Token *consume_enum_tag();
Token *consume_typecore();
Token *consume_ident();
Token *consume_numsuffix();
int consume_incdec();
int consume_compo_assign();
void expect(char *op);
int expect_numeric();
bool at_eof();

// find.c
void error_undef(Token *tok, DefKind kind);
Def *find_def(Token *tok, DefKind kind);
Def *find_enumconst(Token *tok);
Def *fit_def_noerr(Token *tok, DefKind kind);
Def *fit_def(Token *tok, DefKind kind);
bool can_def_symbol(Token *sym);
bool can_def_tag(Token *tag);

// scope.c
void scope_in();
void scope_out();
void args_in();
void args_out();
void member_in();
void member_out();
void loop_in();
void loop_out();
void switch_in();
void switch_out();
#endif  // HEADER_H

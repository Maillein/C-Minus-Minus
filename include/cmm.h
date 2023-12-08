#pragma once

#include <stdbool.h>

struct Type;
struct ASTNode;

//////////////////////
// tokenizer.c
//////////////////////

// トークンの種類
enum TokenKind {
  TK_EXCLA,      // !
  TK_DQUOTE,     // "
  TK_HASH,       // #
  TK_DOLLER,     // $
  TK_PERCENT,    // %
  TK_AMP,        // &
  TK_DAMP,       // &&
  TK_SQUOTE,     // '
  TK_L_PAREN,    // (
  TK_R_PAREN,    // )
  TK_ASTER,      // *
  TK_PLUS,       // +
  TK_DPLUS,      // ++
  TK_COMMA,      // ,
  TK_MINUS,      // -
  TK_DMINUS,     // --
  TK_ALLOW,      // ->
  TK_PERIOD,     // .
  TK_SLASH,      // /
  TK_DSLASH,     // //
  TK_SLA_AST,    // /*
  TK_AST_SLA,    // */
  TK_COLON,      // :
  TK_SEMICOLON,  // ;
  TK_EQUAL,      // =
  TK_DEQUAL,     // ==
  TK_NEQUAL,     // !=
  TK_PLUSEQUAL,  // +=
  TK_MINUSEQUAL, // -=
  TK_LT,         // <
  TK_LE,         // <=
  TK_GT,         // >
  TK_GE,         // >=
  TK_L_SBRACKET, // [
  TK_R_SBRACKET, // ]
  TK_BACKSLASH,  // バックスラッシュ
  TK_CARET,      // ^
  TK_L_CBRACKET, // {
  TK_R_CBRACKET, // }
  TK_VBAR,       // |
  TK_DVBAR,      // ||
  TK_TILDE,      // ~
  TK_USCORE,     // _
  TK_SIZEOF,     // sizeof
  TK_IF,         // if
  TK_ELSE,       // else
  TK_FOR,        // for
  TK_WHILE,      // while
  TK_RETURN,     // return
  TK_INT,        // int
  TK_IDENT,      // 識別子
  TK_INTEGER,    // 整数トークン
  TK_EOF,        // 入力の終わりを表すトークン
};

// トークン型
struct Token {
  enum TokenKind kind; // トークンの型
  struct Token *next;  // 次の入力トークン
  int val;             // kindがTK_NUMの場合，その整数
  char *str;           // トークン文字列
  int len;             // トークンの長さ
};

// ユーザの入力
extern char *user_input;

char *TokenKind2str(enum TokenKind kind);
void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
// struct Token *skip(struct Token **tok, char *op);
bool consume(struct Token **tok, enum TokenKind kind);
bool expect(struct Token **tok, enum TokenKind kind);
bool at_eof(struct Token **tok);
struct Token *tokenize();

//////////////////////
// type.c
//////////////////////

enum TypeKind {
  TY_INT,
  TY_PTR,
  TY_ARRAY,
  TY_FUNCTION,
};

// 式の型
struct Type {
  enum TypeKind kind; // トークンの種類
  int size;           // sizeofの計算に使用
  struct Type *base;  // ポインタ型や配列型で使用

  // 変数宣言で使用
  struct Token *name;

  // kind == ARRAY で使用
  int array_length;

  // kind == FUNC で使用
  struct Type *return_type;
  struct Type *param;
  struct Type *next_param;
};

struct Type *primitive_type(enum TypeKind kind);
struct Type *pointer_to(struct Type *base);
struct Type *array_of(struct Type *base, int array_length);
struct Type *function_type(struct Type *return_type);

struct Type *get_base_type(struct Type *type);
int size_of_type(struct Type *type);
struct Type *solve_node_type(struct ASTNode *node);

//////////////////////
// node.c
//////////////////////

// 抽象構文木のノードの種類
enum NodeKind {
  ND_PROGRAM,   // プログラムの翻訳単位
  ND_ADD,       // +
  ND_SUB,       // -
  ND_MUL,       // *
  ND_DIV,       // /
  ND_MOD,       // %
  ND_EQ,        // ==
  ND_NE,        // !=
  ND_LT,        // <
  ND_LE,        // <=
  ND_OR,        // ||
  ND_AND,       // &&
  ND_ASSIGN,    // 代入
  ND_NUM,       // 整数
  ND_LVAR,      // ローカル変数
  ND_RETURN,    // リターン文
  ND_IF,        // if文
  ND_WHILE,     // while文
  ND_FOR,       // for文
  ND_BLOCK,     // ブロック
  ND_EMPTY,     // 空のブロック
  ND_FUNC_CALL, // 関数呼び出し
  ND_FUNC_ARG,  // 関数の引数
  ND_FUNC_DEF,  // 関数定義
  ND_ADDR,      // アドレス演算
  ND_DEREF,     // 参照
  ND_VAR_DEF,   // 変数定義
};

// 抽象構文木のノード型
struct ASTNode {
  enum NodeKind kind;
  struct ASTNode *lhs;
  struct ASTNode *rhs;
  int val;               // kind == ND_NUMのとき使用
  struct Variable *lvar; // kind == ND_LVARのとき使用
  struct Type *type;     // 型

  struct ASTNode *init;   // kind == ND_FOR のとき使用．for文の初期化
  struct ASTNode *update; // kind == ND_FOR のとき使用．for文の更新
  struct ASTNode *cond;   // kind == ND_IF | ND_WHILE | ND_FOR のとき使用
  struct ASTNode *stmt1;  // kind == ND_IF | ND_WHILE | ND_FOR のとき使用
  struct ASTNode *stmt2;  // kind == ND_IFのとき使用

  char *func_name;   // kind == ND_FUNC_CALL | ND_FUNC_DEF のとき使用
  struct ASTNode *args; // kind == ND_FUNC_CALL のとき使用

  int stack_size;          // kind == ND_FUNC_DEF のとき使用
  int nparams;             // kind == ND_FUNC_DEF のとき使用
  struct Function *func; // kind==ND_FUNC_DEF のとき使用
  struct Variable *params; // kind==ND_FUNC_DEF のとき使用
};

struct ASTNode *new_node(enum NodeKind kind);
struct ASTNode *new_node_binary(enum NodeKind kind, struct ASTNode *lhs,
                             struct ASTNode *rhs);
struct ASTNode *new_node_unary(enum NodeKind kind, struct ASTNode *lhs);
struct ASTNode *new_node_var(struct Variable *lvar, struct Type *type);
struct ASTNode *new_node_num(int val, struct Type *type);
struct ASTNode *new_node_if(struct ASTNode *cond, struct ASTNode *stmt1,
                         struct ASTNode *stmt2);
struct ASTNode *new_node_while(struct ASTNode *cond, struct ASTNode *stmt1);
struct ASTNode *new_node_for(struct ASTNode *for_begin, struct ASTNode *cond,
                          struct ASTNode *for_after, struct ASTNode *stmt1);

//////////////////////
// parser.c
//////////////////////

// 変数の情報
struct Variable {
  struct Variable *prev; // 前の変数
  struct Variable *next; // 次の変数
  char *name;            // 変数名
  int len;               // 変数名の長さ
  int offset;            // RBPからのオフセット
  struct Type *type;     // 型
};

struct Function {
  struct Function *prev;
  struct Function *next;
  char *name; // 変数名
  int len;    // 変数名の長さ
  struct Type *return_type;
  struct Type *params;
};

struct VarList {
  struct Variable *head;
  int len;
};

struct FuncList {
  struct Function *head;
  int len;
};

struct Context {
  struct Context *next;
  struct VarList *locals;
  struct VarList **globals;
  struct FuncList **functions;
  int locals_offset;
  int max_local_offset;
};

struct ASTNode *parse(struct Token **tok);
struct Type *solve_node_type(struct ASTNode *node);

//////////////////////
// codegen.c
//////////////////////

void codegen(struct ASTNode *node);

//////////////////////
// visualizer.c
//////////////////////
void vis_ast(struct ASTNode *node);

#pragma once
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//////////////////////
// tokenizer.c
//////////////////////

// トークンの種類
enum TokenKind {
  TK_OP,    // 記号
  TK_IDENT, // 識別子
  TK_NUM,   // 整数トークン
  TK_KEYWD, // 予約語
  TK_EOF,   // 入力の終わりを表すトークン
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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool equal(struct Token **tok, char *op);
bool check(struct Token **tok, char *op);
struct Token *skip(struct Token **tok, char *op);
bool consume(struct Token **tok, enum TokenKind kind);
bool at_eof(struct Token **tok);
struct Token *new_token(enum TokenKind kind, struct Token *cur, char *str,
                        int len);
struct Token *tokenize();

//////////////////////
// parser.c
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

enum TypeKind {
  INT,
  PTR,
};

// 式の型
struct Type {
  enum TypeKind kind;
  struct Type *ptr_to;
};

// ローカル変数の情報
struct LVar {
  struct LVar *next; // 次の変数
  char *name;        // 変数名
  int len;           // 変数名の長さ
  int offset;        // RBPからのオフセット
  struct Type *type; // 型
};

struct Context {
  struct Context *next;
  struct LVar *locals;
  int locals_offset;
  int max_local_offset;
};

// 抽象構文木のノード型
struct Node {
  enum NodeKind kind;
  struct Node *lhs;
  struct Node *rhs;
  int val;           // kind == ND_NUMのとき使用
  struct LVar *lvar; // kind == ND_LVARのとき使用
  struct Type *type; // 型

  struct Node *init;   // kind == ND_FOR のとき使用．for文の初期化
  struct Node *update; // kind == ND_FOR のとき使用．for文の更新
  struct Node *cond;   // kind == ND_IF | ND_WHILE | ND_FOR のとき使用
  struct Node *stmt1;  // kind == ND_IF | ND_WHILE | ND_FOR のとき使用
  struct Node *stmt2;  // kind == ND_IFのとき使用

  char *func_name;   // kind == ND_FUNC_CALL | ND_FUNC_DEF のとき使用
  struct Node *args; // kind == ND_FUNC_CALL のとき使用

  int stack_size;      // kind == ND_FUNC_DEF のとき使用
  int nparams;         // kind == ND_FUNC_DEF のとき使用
  struct LVar *params; // kind==ND_FUNC_DEF のとき使用
};

// プログラム全体
extern struct Node *code[100];

struct Node *parse(struct Token **tok);

//////////////////////
// codegen.c
//////////////////////

void codegen(struct Node *node);

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
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // 代入
  ND_NUM,    // 整数
  ND_LVAR,   // ローカル変数
  ND_RETURN, // リターン文
  ND_IF,     // if文
};

// 抽象構文木のノード型
struct Node {
  enum NodeKind kind;
  struct Node *lhs;
  struct Node *rhs;
  int val;    // kind == ND_NUMのとき使用
  int offset; // kind == ND_LVARのとき使用
};

struct Node *new_node(enum NodeKind kind, struct Node *lhs, struct Node *rhs);
struct Node *new_node_val(int offset);
struct Node *new_node_num(int val);

// プログラム全体
extern struct Node *code[100];

// ローカル変数の型
struct LVar {
  struct LVar *next; // 次の変数
  char *name;        // 変数名
  int len;           // 変数名の長さ
  int offset;        // RBPからのオフセット
};

// struct LVar *find_lvar();
// struct LVar *new_lvar();

void program(struct Token **tok);
struct Node *stmt(struct Token **tok);
struct Node *expr(struct Token **tok);
struct Node *assign(struct Token **tok);
struct Node *equality(struct Token **tok);
struct Node *relational(struct Token **tok);
struct Node *add(struct Token **tok);
struct Node *mul(struct Token **tok);
struct Node *unary(struct Token **tok);
struct Node *primary(struct Token **tok);

//////////////////////
// codegen.c
//////////////////////

void gen_lval(struct Node *node);
void gen(struct Node *node);
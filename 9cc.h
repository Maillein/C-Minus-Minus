#pragma once
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
struct Token {
  TokenKind kind;     // トークンの型
  struct Token *next; // 次の入力トークン
  int val;            // kindがTK_NUMの場合，その整数
  char *str;          // トークン文字列
  int len;            // トークンの長さ
};

// ユーザの入力
extern char *user_input;

// 現在着目しているトークン
extern struct Token *token;

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
void expect(char *op);
int expect_number();
bool at_eof();
struct Token *new_token(TokenKind kind, struct Token *cur, char *str, int len);
struct Token *tokenize();

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // 整数
} NodeKind;

// 抽象構文木のノード型
struct Node {
  NodeKind kind;
  struct Node *lhs;
  struct Node *rhs;
  int val;
};

struct Node *new_node(NodeKind kind, struct Node *lhs, struct Node *rhs);
struct Node *new_node_num(int val);
struct Node *expr();
struct Node *equality();
struct Node *relational();
struct Node *add();
struct Node *mul();
struct Node *unary();
struct Node *primary();
void gen(struct Node *node);

#include "cmm.h"
#include <stdlib.h>

struct ASTNode *new_node(enum NodeKind kind) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = kind;
  return node;
}

struct ASTNode *new_node_binary(enum NodeKind kind, struct ASTNode *lhs,
                             struct ASTNode *rhs) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct ASTNode *new_node_unary(enum NodeKind kind, struct ASTNode *lhs) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = kind;
  node->lhs = lhs;
  return node;
}

struct ASTNode *new_node_var(struct Variable *lvar, struct Type *type) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = ND_LVAR;
  node->lvar = lvar;
  node->type = type;
  return node;
}

struct ASTNode *new_node_num(int val, struct Type *type) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = ND_NUM;
  node->val = val;
  node->type = type;
  return node;
}

struct ASTNode *new_node_if(struct ASTNode *cond, struct ASTNode *stmt1,
                         struct ASTNode *stmt2) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = ND_IF;
  node->cond = cond;
  node->stmt1 = stmt1;
  node->stmt2 = stmt2;
  return node;
}

struct ASTNode *new_node_while(struct ASTNode *cond, struct ASTNode *stmt1) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = ND_WHILE;
  node->cond = cond;
  node->stmt1 = stmt1;
  return node;
}

struct ASTNode *new_node_for(struct ASTNode *for_begin, struct ASTNode *cond,
                          struct ASTNode *for_after, struct ASTNode *stmt1) {
  struct ASTNode *node = calloc(1, sizeof(struct ASTNode));
  node->kind = ND_FOR;
  node->init = for_begin;
  node->cond = cond;
  node->update = for_after;
  node->stmt1 = stmt1;
  return node;
}

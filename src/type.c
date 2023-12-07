#include "cmm.h"
#include <assert.h>
#include <stdlib.h>

char *TypeKind2str(enum TypeKind kind) {
  switch (kind) {
  case INT:
    return "INT";
  case PTR:
    return "PTR";
  // case ARRAY:
  //   return "ARRAY";
  }
  return NULL;
}

struct Type *primitive_type(enum TypeKind kind) {
  struct Type *type = calloc(1, sizeof(struct Type));
  type->kind = kind;
  switch (kind) {
  case INT:
    type->size = 4;
    break;
  case PTR:
  // case ARRAY:
    error("プリミティブ型ではありません：%s", TypeKind2str(kind));
    break;
  }
  return type;
}

struct Type *pointer_to(struct Type *base) {
  struct Type *type = calloc(1, sizeof(struct Type));
  type->kind = PTR;
  type->size = 8;
  type->base = base;
  return type;
}

// struct Type *array_of(struct Type *base, int array_length) {
//   struct Type *type = calloc(1, sizeof(struct Type));
//   type->kind = ARRAY;
//   type->size = array_length * base->size;
//   type->base = base;
//   return type;
// }

struct Type *solve_node_type(struct Node *node) {
  if (node == NULL) {
    return NULL;
  }

  switch (node->kind) {
  case ND_ADD:
  case ND_SUB: {
    struct Type *type_l = solve_node_type(node->lhs);
    struct Type *type_r = solve_node_type(node->rhs);
    if (type_l->kind == type_r->kind || type_l->kind == PTR) {
      return node->type = type_l;
    }
    assert(0 && "加減算の両辺の型が不正です");
  } break;
  case ND_MUL:
  case ND_DIV:
  case ND_MOD:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
  case ND_OR:
  case ND_AND:
  case ND_ASSIGN: {
    struct Type *type_l = solve_node_type(node->lhs);
    struct Type *type_r = solve_node_type(node->rhs);
    assert(type_l->kind == type_r->kind && "両辺の型が一致していません");
    return node->type = type_l;
  }
  case ND_NUM:
  case ND_LVAR:
    return node->type;
  case ND_FUNC_CALL: {
    int nargs = 0;
    for (struct Node *arg = node->args; arg && nargs <= 6; arg = arg->rhs) {
      solve_node_type(arg);
      nargs++;
    }
    return node->type;
  }
  case ND_RETURN:
    solve_node_type(node->lhs);
    return NULL;
  case ND_IF:
    solve_node_type(node->cond);
    solve_node_type(node->stmt1);
    solve_node_type(node->stmt2);
    return NULL;
  case ND_WHILE:
    solve_node_type(node->cond);
    solve_node_type(node->stmt1);
    return NULL;
  case ND_FOR:
    solve_node_type(node->init);
    solve_node_type(node->cond);
    solve_node_type(node->update);
    solve_node_type(node->stmt1);
    return NULL;
  case ND_BLOCK:
    for (; node; node = node->rhs) {
      solve_node_type(node->lhs);
    }
    return NULL;
  case ND_FUNC_DEF:
    solve_node_type(node->rhs);
    return NULL;
  case ND_ADDR:
    node->type = calloc(1, sizeof(struct Type));
    node->type->kind = PTR;
    node->type->size = 8;
    node->type->base = solve_node_type(node->lhs);
    return node->type;
  case ND_DEREF:
    if (node->lhs->type == NULL) {
      solve_node_type(node->lhs);
    }
    return node->type = node->lhs->type->base;
  case ND_FUNC_ARG:
    return node->type = solve_node_type(node->lhs);
  case ND_PROGRAM:
  case ND_EMPTY:
  case ND_VAR_DEF:
    return NULL;
  }

  return NULL;
}

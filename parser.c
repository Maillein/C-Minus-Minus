#include "cmm.h"
#include <stdlib.h>

struct Node *new_node(enum NodeKind kind, struct Node *lhs, struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_val(int offset) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_LVAR;
  node->offset = offset;
  return node;
}

struct Node *new_node_num(int val) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

struct Node *code[100];
struct LVar *locals;

struct LVar *find_lvar(struct Token **tok) {
  for (struct LVar *var = locals; var; var = var->next) {
    if (var->len == (*tok)->len &&
        !memcmp((*tok)->str, var->name, (*tok)->len)) {
      return var;
    }
  }
  return NULL;
}

struct LVar *new_lvar(struct Token **tok) {
  struct LVar *lvar = calloc(1, sizeof(struct LVar));
  lvar->next = locals;
  lvar->name = (*tok)->str;
  lvar->len = (*tok)->len;
  lvar->offset = locals ? locals->offset + 8 : 8;
  locals = lvar;
  return lvar;
}

// program = stmt*
void program(struct Token **tok) {
  int i = 0;
  while (!at_eof(tok)) {
    code[i++] = stmt(tok);
  }
  code[i] = NULL;
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt
struct Node *stmt(struct Token **tok) {
  if (equal(tok, "if")) {
    *tok = skip(tok, "(");
    struct Node *lhs = expr(tok);
    *tok = skip(tok, ")");
    struct Node *rhs = stmt(tok);
    return new_node(ND_IF, lhs, rhs);
  } else if (equal(tok, "return")) {
    struct Node *node = expr(tok);
    *tok = skip(tok, ";");
    return new_node(ND_RETURN, node, NULL);
  } else {
    struct Node *node = expr(tok);
    *tok = skip(tok, ";");
    return node;
  }
}

// expr = assign
struct Node *expr(struct Token **tok) { return assign(tok); }

// assign  = equality ( "=" assign )?
struct Node *assign(struct Token **tok) {
  struct Node *node = equality(tok);
  if (equal(tok, "=")) {
    node = new_node(ND_ASSIGN, node, assign(tok));
  }
  return node;
}

// equality = relational ( "==" relational | "!=" relational )*
struct Node *equality(struct Token **tok) {
  struct Node *node = relational(tok);

  for (;;) {
    if (equal(tok, "==")) {
      node = new_node(ND_EQ, node, relational(tok));
    } else if (equal(tok, "!=")) {
      node = new_node(ND_NE, node, relational(tok));
    } else {
      return node;
    }
  }
}

// relational = add ( "<" add | "<=" add | ">" add | ">=" add )*
struct Node *relational(struct Token **tok) {
  struct Node *node = add(tok);

  for (;;) {
    if (equal(tok, "<")) {
      node = new_node(ND_LT, node, add(tok));
    } else if (equal(tok, "<=")) {
      node = new_node(ND_LE, node, add(tok));
    } else if (equal(tok, ">")) {
      node = new_node(ND_LT, add(tok), node);
    } else if (equal(tok, ">=")) {
      node = new_node(ND_LE, add(tok), node);
    } else {
      return node;
    }
  }
}

// add = mul ( "+" mul | "-" mul )*
struct Node *add(struct Token **tok) {
  struct Node *node = mul(tok);

  for (;;) {
    if (equal(tok, "+")) {
      node = new_node(ND_ADD, node, mul(tok));
    } else if (equal(tok, "-")) {
      node = new_node(ND_SUB, node, mul(tok));
    } else {
      return node;
    }
  }
}

// mul = unary ( "*" unary | "/" unary )*
struct Node *mul(struct Token **tok) {
  struct Node *node = unary(tok);

  for (;;) {
    if (equal(tok, "*")) {
      node = new_node(ND_MUL, node, unary(tok));
    } else if (equal(tok, "/")) {
      node = new_node(ND_DIV, node, unary(tok));
    } else {
      return node;
    }
  }
}

// unary = ( "+" | "-" )? primary
struct Node *unary(struct Token **tok) {
  if (equal(tok, "+")) {
    return primary(tok);
  } else if (equal(tok, "-")) {
    return new_node(ND_SUB, new_node_num(0), primary(tok));
  } else {
    return primary(tok);
  }
}

// primary = num | ident | "(" expr ")"
struct Node *primary(struct Token **tok) {
  if (equal(tok, "(")) {
    struct Node *node = expr(tok);
    *tok = skip(tok, ")");
    return node;
  } else if ((*tok)->kind == TK_IDENT) {
    struct LVar *lvar = find_lvar(tok);
    if (lvar == NULL) {
      lvar = new_lvar(tok);
    }
    consume(tok, TK_IDENT);
    return new_node_val(lvar->offset);
  } else if ((*tok)->kind == TK_NUM) {
    struct Node *node = new_node_num((*tok)->val);
    consume(tok, TK_NUM);
    return node;
  }

  error("予期しないトークンです: %s", (*tok)->str);
}
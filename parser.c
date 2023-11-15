#include "9cc.h"

struct Node *new_node(NodeKind kind, struct Node *lhs, struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_num(int val) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

// expr = equality
struct Node *expr() { return equality(); }

// equality = relational ( "==" relational | "!=" relational )*
struct Node *equality() {
  struct Node *node = relational();

  for (;;) {
    if (consume("==")) {
      node = new_node(ND_EQ, node, relational());
    } else if (consume("!=")) {
      node = new_node(ND_NE, node, relational());
    } else {
      return node;
    }
  }
}

// relational = add ( "<" add | "<=" add | ">" add | ">=" add )*
struct Node *relational() {
  struct Node *node = add();

  for (;;) {
    if (consume("<")) {
      node = new_node(ND_LT, node, add());
    } else if (consume("<=")) {
      node = new_node(ND_LE, node, add());
    } else if (consume(">")) {
      node = new_node(ND_LT, add(), node);
    } else if (consume(">=")) {
      node = new_node(ND_LE, add(), node);
    } else {
      return node;
    }
  }
}

// add = mul ( "+" mul | "-" mul )*
struct Node *add() {
  struct Node *node = mul();

  for (;;) {
    if (consume("+")) {
      node = new_node(ND_ADD, node, mul());
    } else if (consume("-")) {
      node = new_node(ND_SUB, node, mul());
    } else {
      return node;
    }
  }
}

// mul = unary ( "*" unary | "/" unary )*
struct Node *mul() {
  struct Node *node = unary();

  for (;;) {
    if (consume("*")) {
      node = new_node(ND_MUL, node, unary());
    } else if (consume("/")) {
      node = new_node(ND_DIV, node, unary());
    } else {
      return node;
    }
  }
}

// unary = ( "+" | "-" )? primary
struct Node *unary() {
  if (consume("+")) {
    return primary();
  } else if (consume("-")) {
    return new_node(ND_SUB, new_node_num(0), primary());
  } else {
    return primary();
  }
}

// primary = num | "(" expr ")"
struct Node *primary() {
  struct Node *node = NULL;
  if (consume("(")) {
    node = expr();
    expect(")");
  } else {
    node = new_node_num(expect_number());
  }
  return node;
}

#include "cmm.h"
#include <stdlib.h>

struct Node *new_node(enum NodeKind kind, struct Node *lhs, struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_var(int offset) {
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

struct Node *new_node_if(struct Node *cond, struct Node *stmt1,
                         struct Node *stmt2) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_IF;
  node->cond = cond;
  node->stmt1 = stmt1;
  node->stmt2 = stmt2;
  return node;
}

struct Node *new_node_while(struct Node *cond, struct Node *stmt1) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_WHILE;
  node->cond = cond;
  node->stmt1 = stmt1;
  return node;
}

struct Node *new_node_for(struct Node *for_begin, struct Node *cond,
                          struct Node *for_after, struct Node *stmt1) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_FOR;
  node->for_begin = for_begin;
  node->cond = cond;
  node->for_after = for_after;
  node->stmt1 = stmt1;
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

// program = func_definition
struct Node *parse(struct Token **tok) {
  return func_definition(tok);
}

// func_definition = ident "(" ( ident ( "," ident )* )? ")" "{" stmt* "}"
struct Node *func_definition(struct Token **tok) {
  if ((*tok)->kind != TK_IDENT) {
    error_at((*tok)->str, "関数定義ではありません．");
  }
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_FUNC_DEF;
  node->func_name = calloc((*tok)->len + 1, sizeof(char));
  memcpy(node->func_name, (*tok)->str, (*tok)->len);
  consume(tok, TK_IDENT);
  *tok = skip(tok, "(");
  // struct Node arg_head;
  // arg_head.rhs = NULL;
  // struct Node *cur = &arg_head;
  while (!equal(tok, ")")) {
  }
  if (!check(tok, "{")) {
    error_at((*tok)->str, "'{'ではありません．");
  }
  node->rhs = stmt(tok);
  return node;
}


// stmt = expr? ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ( "else" stmt )?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
struct Node *stmt(struct Token **tok) {
  if (equal(tok, "if")) {
    struct Node *cond, *stmt1, *stmt2;
    *tok = skip(tok, "(");
    cond = expr(tok);
    *tok = skip(tok, ")");
    stmt1 = stmt(tok);
    if (equal(tok, "else")) {
      stmt2 = stmt(tok);
    } else {
      stmt2 = NULL;
    }
    return new_node_if(cond, stmt1, stmt2);
  }
  if (equal(tok, "while")) {
    struct Node *cond, *stmt1;
    *tok = skip(tok, "(");
    cond = expr(tok);
    *tok = skip(tok, ")");
    stmt1 = stmt(tok);
    return new_node_while(cond, stmt1);
  }
  if (equal(tok, "for")) {
    struct Node *for_begin, *for_after, *cond, *stmt1;
    *tok = skip(tok, "(");
    if (equal(tok, ";")) {
      for_begin = NULL;
    } else {
      for_begin = expr(tok);
      *tok = skip(tok, ";");
    }
    if (equal(tok, ";")) {
      cond = NULL;
    } else {
      cond = expr(tok);
      *tok = skip(tok, ";");
    }
    if (equal(tok, ")")) {
      for_after = NULL;
    } else {
      for_after = expr(tok);
      *tok = skip(tok, ")");
    }
    stmt1 = stmt(tok);
    return new_node_for(for_begin, cond, for_after, stmt1);
  }
  if (equal(tok, "return")) {
    struct Node *node = expr(tok);
    *tok = skip(tok, ";");
    return new_node(ND_RETURN, node, NULL);
  }
  if (equal(tok, "{")) {
    struct Node head;
    head.rhs = NULL;
    struct Node *cur = &head;
    while (!equal(tok, "}")) {
      cur = cur->rhs = new_node(ND_BLOCK, stmt(tok), NULL);
    }
    return head.rhs;
  }
  if (equal(tok, ";")) {
    return new_node(ND_EMPTY, NULL, NULL);
  }
  struct Node *node = expr(tok);
  *tok = skip(tok, ";");
  return node;
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

// primary = num
//         | ident ( "(" ( expr ( "," expr )* )? ")" )
//         | "(" expr ")"
struct Node *primary(struct Token **tok) {
  if (equal(tok, "(")) {
    struct Node *node = expr(tok);
    *tok = skip(tok, ")");
    return node;
  } else if ((*tok)->kind == TK_IDENT) {
    if (equal(&(*tok)->next, "(")) {
      // 関数名をパース
      struct Node *node = new_node(ND_FUNC_CALL, NULL, NULL);
      node->func_name = calloc((*tok)->len + 1, sizeof(char));
      memcpy(node->func_name, (*tok)->str, (*tok)->len * sizeof(char));
      consume(tok, TK_IDENT);
      
      //引数をパース
      struct Node head;
      head.rhs = NULL;
      struct Node *cur = &head;
      while (!equal(tok, ")")) {
        cur = cur->rhs = new_node(ND_FUNC_ARG, expr(tok), NULL);
        equal(tok, ",");
      }
      node->args = head.rhs;

      return node;
    } else {
      struct LVar *lvar = find_lvar(tok);
      if (lvar == NULL) {
        lvar = new_lvar(tok);
      }
      consume(tok, TK_IDENT);
      return new_node_var(lvar->offset);
    }
  } else if ((*tok)->kind == TK_NUM) {
    struct Node *node = new_node_num((*tok)->val);
    consume(tok, TK_NUM);
    return node;
  }

  error("予期しないトークンです: %s", (*tok)->str);
}

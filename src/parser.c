#include "cmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void context_push(struct Context **head) {
  struct Context *context = calloc(1, sizeof(struct Context));
  context->next = *head;
  context->locals_offset = (*head)->locals_offset;
  *head = context;
}
void context_pop(struct Context **head) {
  if ((*head)->next->max_local_offset < (*head)->max_local_offset) {
    (*head)->next->max_local_offset = (*head)->max_local_offset;
  }
  *head = (*head)->next;
}

struct Node *new_node(enum NodeKind kind) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  return node;
}

struct Node *new_node_binary(enum NodeKind kind, struct Node *lhs,
                             struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_unary(enum NodeKind kind, struct Node *lhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  return node;
}

struct Node *new_node_var(struct LVar *lvar, struct Type *type) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_LVAR;
  node->lvar = lvar;
  node->type = type;
  return node;
}

struct Node *new_node_num(int val, struct Type *type) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = type;
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
  node->init = for_begin;
  node->cond = cond;
  node->update = for_after;
  node->stmt1 = stmt1;
  return node;
}

struct LVar *find_lvar(struct Token **tok, struct Context **context) {
  for (struct Context *cur = *context; cur; cur = cur->next) {
    for (struct LVar *var = cur->locals; var; var = var->next) {
      if (var->len == (*tok)->len &&
          !memcmp((*tok)->str, var->name, (*tok)->len)) {
        return var;
      }
    }
  }
  return NULL;
}

struct LVar *new_lvar(struct Token **tok, struct Context **context,
                      struct Type *type) {
  struct LVar *lvar = calloc(1, sizeof(struct LVar));
  lvar->next = (*context)->locals;
  lvar->name = calloc((*tok)->len + 1, sizeof(char));
  lvar->len = (*tok)->len;
  memcpy(lvar->name, (*tok)->str, lvar->len);
  lvar->type = type;
  lvar->offset = (*context)->locals_offset =
      (*context)->locals_offset + type->size;
  if ((*context)->max_local_offset < (*context)->locals_offset) {
    (*context)->max_local_offset = (*context)->locals_offset;
  }
  (*context)->locals = lvar;
  return lvar;
}

struct LVar *find_function(struct Token **tok, struct Context **context) {
  for (struct Context *cur = *context; cur; cur = cur->next) {
    for (struct LVar *var = cur->functions; var; var = var->next) {
      if (var->len == (*tok)->len &&
          !memcmp((*tok)->str, var->name, (*tok)->len)) {
        return var;
      }
    }
  }
  return NULL;
}

struct LVar *new_function(struct Token **tok, struct Context **context,
                          struct Type *type) {
  struct LVar *lvar = calloc(1, sizeof(struct LVar));
  lvar->next = (*context)->functions;
  lvar->name = calloc((*tok)->len + 1, sizeof(char));
  lvar->len = (*tok)->len;
  memcpy(lvar->name, (*tok)->str, lvar->len);
  lvar->type = type;
  (*context)->functions = lvar;
  return lvar;
}

struct Node *func_definition(struct Token **tok, struct Context **context);
struct Node *stmt(struct Token **tok, struct Context **context);
struct Node *expr(struct Token **tok, struct Context **context);
struct Node *assign(struct Token **tok, struct Context **context);
struct Node *equality(struct Token **tok, struct Context **context);
struct Node *relational(struct Token **tok, struct Context **context);
struct Node *add(struct Token **tok, struct Context **context);
struct Node *mul(struct Token **tok, struct Context **context);
struct Node *unary(struct Token **tok, struct Context **context);
struct Node *primary(struct Token **tok, struct Context **context);
struct Node *var_delaration(struct Token **tok, struct Context **context);

// program = func_definition*
struct Node *parse(struct Token **tok) {
  struct Node *node = new_node(ND_PROGRAM);
  struct Context *context = calloc(1, sizeof(struct Context));
  struct Node *cur = node;
  while (!at_eof(tok)) {
    cur = cur->rhs = new_node(ND_PROGRAM);
    cur->lhs = func_definition(tok, &context);
  }
  return node->rhs;
}

// func_definition = "int" ident "(" ( "int" ident ( "," "int" ident )* )? ")" ( ";" | "{" stmt* "}" )
struct Node *func_definition(struct Token **tok, struct Context **context) {
  expect(tok, TK_INT); // 関数の返り値は必ずint
  // 関数名をパース
  if ((*tok)->kind != TK_IDENT) {
    error_at((*tok)->str, "関数定義ではありません．");
  }
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_FUNC_DEF;
  node->func_name = calloc((*tok)->len + 1, sizeof(char));
  memcpy(node->func_name, (*tok)->str, (*tok)->len);
  struct Type *type = calloc(1, sizeof(struct Type));
  type->kind = INT;
  type->size = 4;
  new_function(tok, context, type);
  expect(tok, TK_IDENT);

  // 関数の引数をパース
  context_push(context);
  expect(tok, TK_L_PAREN);
  while (!consume(tok, TK_R_PAREN)) {
    var_delaration(tok, context);
    node->nparams++;
    consume(tok, TK_COMMA);
  }
  node->params = (*context)->locals;

  if (consume(tok, TK_SEMICOLON)) {
    context_pop(context);
    node->stack_size = (*context)->max_local_offset;
    return node;
  }

  // 関数の本体をパース
  // if (!check(tok, "{")) {
  //   error_at((*tok)->str, "'{'ではありません．");
  // }
  node->rhs = stmt(tok, context);

  context_pop(context);
  node->stack_size = (*context)->max_local_offset;
  return node;
}

// stmt = expr? ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ( "else" stmt )?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | var_delaration ";"
struct Node *stmt(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_IF)) {
    struct Node *cond, *stmt1, *stmt2;
    expect(tok, TK_L_PAREN);
    cond = expr(tok, context);
    expect(tok, TK_R_PAREN);
    stmt1 = stmt(tok, context);
    if (consume(tok, TK_ELSE)) {
      stmt2 = stmt(tok, context);
    } else {
      stmt2 = NULL;
    }
    return new_node_if(cond, stmt1, stmt2);
  }

  if (consume(tok, TK_WHILE)) {
    struct Node *cond, *stmt1;
    expect(tok, TK_L_PAREN);
    cond = expr(tok, context);
    expect(tok, TK_R_PAREN);
    stmt1 = stmt(tok, context);
    return new_node_while(cond, stmt1);
  }

  if (consume(tok, TK_FOR)) {
    context_push(context);
    struct Node *for_begin, *for_after, *cond, *stmt1;
    expect(tok, TK_L_PAREN);
    if (consume(tok, TK_SEMICOLON)) {
      for_begin = NULL;
    } else {
      for_begin = expr(tok, context);
      expect(tok, TK_SEMICOLON);
    }

    if (consume(tok, TK_SEMICOLON)) {
      cond = NULL;
    } else {
      cond = expr(tok, context);
      expect(tok, TK_SEMICOLON);
    }

    if (consume(tok, TK_R_PAREN)) {
      for_after = NULL;
    } else {
      for_after = expr(tok, context);
      expect(tok, TK_R_PAREN);
    }
    stmt1 = stmt(tok, context);
    context_pop(context);
    return new_node_for(for_begin, cond, for_after, stmt1);
  }

  if (consume(tok, TK_RETURN)) {
    struct Node *node = expr(tok, context);
    expect(tok, TK_SEMICOLON);
    return new_node_unary(ND_RETURN, node);
  }

  if (consume(tok, TK_L_CBRACKET)) {
    struct Node head;
    head.rhs = NULL;
    struct Node *cur = &head;
    context_push(context);
    while (!consume(tok, TK_R_CBRACKET)) {
      cur = cur->rhs = new_node_unary(ND_BLOCK, stmt(tok, context));
    }
    context_pop(context);
    return head.rhs;
  }

  if ((*tok)->kind == TK_INT) {
    struct Node *node = var_delaration(tok, context);
    expect(tok, TK_SEMICOLON);
    return node;
  }

  if (!consume(tok, TK_SEMICOLON)) {
    struct Node *node = expr(tok, context);
    expect(tok, TK_SEMICOLON);
    return node;
  }

  while (consume(tok, TK_SEMICOLON));
  return new_node(ND_EMPTY); // unreachable
}

// expr = assign ( "==" assign | "&&" assign )*
struct Node *expr(struct Token **tok, struct Context **context) {
  struct Node *node = assign(tok, context);
  for (;;) {
    if (consume(tok, TK_DVBAR)) {
      node = new_node_binary(ND_OR, node, assign(tok, context));
    } else if (consume(tok, TK_DAMP)) {
      node = new_node_binary(ND_AND, node, assign(tok, context));
    } else {
      return node;
    }
  }
}

// assign  = equality ( "=" assign )?
struct Node *assign(struct Token **tok, struct Context **context) {
  struct Node *node = equality(tok, context);
  if (consume(tok, TK_EQUAL)) {
    node = new_node_binary(ND_ASSIGN, node, assign(tok, context));
  }
  return node;
}

// equality = relational ( "==" relational | "!=" relational )*
struct Node *equality(struct Token **tok, struct Context **context) {
  struct Node *node = relational(tok, context);

  for (;;) {
    if (consume(tok, TK_DEQUAL)) {
      node = new_node_binary(ND_EQ, node, relational(tok, context));
    } else if (consume(tok, TK_NEQUAL)) {
      node = new_node_binary(ND_NE, node, relational(tok, context));
    } else {
      return node;
    }
  }
}

// relational = add ( "<" add | "<=" add | ">" add | ">=" add )*
struct Node *relational(struct Token **tok, struct Context **context) {
  struct Node *node = add(tok, context);

  for (;;) {
    if (consume(tok, TK_LT)) {
      node = new_node_binary(ND_LT, node, add(tok, context));
    } else if (consume(tok, TK_LE)) {
      node = new_node_binary(ND_LE, node, add(tok, context));
    } else if (consume(tok, TK_GT)) {
      node = new_node_binary(ND_LT, add(tok, context), node);
    } else if (consume(tok, TK_GE)) {
      node = new_node_binary(ND_LE, add(tok, context), node);
    } else {
      return node;
    }
  }
}

// add = mul ( "+" mul | "-" mul )*
struct Node *add(struct Token **tok, struct Context **context) {
  struct Node *node = mul(tok, context);

  for (;;) {
    if (consume(tok, TK_PLUS)) {
      node = new_node_binary(ND_ADD, node, mul(tok, context));
    } else if (consume(tok, TK_MINUS)) {
      node = new_node_binary(ND_SUB, node, mul(tok, context));
    } else {
      return node;
    }
  }
}

// mul = unary ( "*" unary | "/" unary | "%" unary )*
struct Node *mul(struct Token **tok, struct Context **context) {
  struct Node *node = unary(tok, context);

  for (;;) {
    if (consume(tok, TK_ASTER)) {
      node = new_node_binary(ND_MUL, node, unary(tok, context));
    } else if (consume(tok, TK_SLASH)) {
      node = new_node_binary(ND_DIV, node, unary(tok, context));
    } else if (consume(tok, TK_PERCENT)) {
      node = new_node_binary(ND_MOD, node, unary(tok, context));
    } else {
      return node;
    }
  }
}

// unary = ( "+" | "-" )? primary
//       | "sizeof" primary
//       | "&" unary
//       | "*" unary
struct Node *unary(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_PLUS)) {
    return primary(tok, context);
  } else if (consume(tok, TK_MINUS)) {
    return new_node_binary(ND_SUB, new_node_num(0, primitive_type(INT)),
                           primary(tok, context));
  } else if (consume(tok, TK_AMP)) {
    return new_node_unary(ND_ADDR, unary(tok, context));
  } else if (consume(tok, TK_ASTER)) {
    return new_node_unary(ND_DEREF, unary(tok, context));
  } else if (consume(tok, TK_SIZEOF)) {
    struct Node *node = primary(tok, context);
    solve_node_type(node);
    return new_node_num(node->type->size, primitive_type(INT));
  } else {
    return primary(tok, context);
  }
}

// primary = num
//         | ident ( "(" ( expr ( "," expr )* )? ")" )
//         | "(" expr ")"
struct Node *primary(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_L_PAREN)) {
    struct Node *node = expr(tok, context);
    expect(tok, TK_R_PAREN);
    return node;
  } else if ((*tok)->kind == TK_IDENT) {
    if ((*tok)->next->kind == TK_L_PAREN) { // 関数呼び出しのパース
      // 関数名をパース
      struct Node *node = new_node_binary(ND_FUNC_CALL, NULL, NULL);
      node->func_name = calloc((*tok)->len + 1, sizeof(char));
      memcpy(node->func_name, (*tok)->str, (*tok)->len * sizeof(char));
      struct LVar *func = find_function(tok, context);
      if (func == NULL) {
        error_at((*tok)->str, "未定義の関数です");
      } else {
        node->type = func->type;
      }
      expect(tok, TK_IDENT);
      expect(tok, TK_L_PAREN);

      // 引数をパース
      struct Node head;
      head.rhs = NULL;
      struct Node *cur = &head;
      while (!consume(tok, TK_R_PAREN)) {
        cur = cur->rhs = new_node_binary(ND_FUNC_ARG, expr(tok, context), NULL);
        consume(tok, TK_COMMA);
      }
      node->args = head.rhs;

      return node;
    } else { // 変数のパース
      struct LVar *lvar = find_lvar(tok, context);
      if (lvar == NULL) {
        error_at((*tok)->str, "未定義の変数です");
        // lvar = new_lvar(tok, context);
      }
      consume(tok, TK_IDENT);
      return new_node_var(lvar, lvar->type);
    }
  } else if ((*tok)->kind == TK_INTEGER) {
    struct Node *node = new_node_num((*tok)->val, primitive_type(INT));
    expect(tok, TK_INTEGER);
    return node;
  }

  error_at((*tok)->str, "予期しないトークンです: %s", TokenKind2str((*tok)->kind));
  return NULL; // unreachable
}

struct Node *var_delaration(struct Token **tok, struct Context **context) {
  expect(tok, TK_INT);
  struct Type *type = primitive_type(INT);
  while (consume(tok, TK_ASTER)) {
    struct Type *t = calloc(1, sizeof(struct Type));
    t->kind = PTR;
    t->size = 8;
    t->base = type;
    type = t;
  }
  new_lvar(tok, context, type);
  expect(tok, TK_IDENT);
  return new_node(ND_VAR_DEF);
}

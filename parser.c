#include "cmm.h"
#include <stdlib.h>

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

struct Node *code[100];

// 未宣言の変数かどうかの区別がまだない．そのため，
// { a = 1; { a = 2; } }
// をパースすると，"a = 2;"が
//   - 初めのaに2を代入
//   - 新たな変数を定義して2を代入
// の区別がつかない．
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
  lvar->offset = (*context)->locals_offset = (*context)->locals_offset + 8;
  lvar->type = type;
  if ((*context)->max_local_offset < (*context)->locals_offset) {
    (*context)->max_local_offset = (*context)->locals_offset;
  }
  (*context)->locals = lvar;
  return lvar;
}

struct Type *new_type(enum TypeKind kind, struct Type *ptr_to) {
  struct Type *type = calloc(1, sizeof(struct Type));
  type->kind = kind;
  type->ptr_to = ptr_to;
  return type;
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
  struct Node *head = new_node(ND_PROGRAM);
  struct Node **tail = &head;
  while (!at_eof(tok)) {
    (*tail)->rhs = new_node(ND_PROGRAM);
    tail = &(*tail)->rhs;
    struct Context *cont_head = calloc(1, sizeof(struct Context));
    struct Context **context = &cont_head;
    (*tail)->lhs = func_definition(tok, context);
  }
  return head->rhs;
}

// func_definition = "int" ident "(" ( "int" ident ( "," "int" ident )* )? ")"
// "{" stmt* "}"
struct Node *func_definition(struct Token **tok, struct Context **context) {
  *tok = skip(tok, "int"); // 関数の返り値は必ずint
  // 関数名をパース
  if ((*tok)->kind != TK_IDENT) {
    error_at((*tok)->str, "関数定義ではありません．");
  }
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_FUNC_DEF;
  node->func_name = calloc((*tok)->len + 1, sizeof(char));
  memcpy(node->func_name, (*tok)->str, (*tok)->len);
  consume(tok, TK_IDENT);

  // 関数の引数をパース
  *tok = skip(tok, "(");
  context_push(context);
  while (!equal(tok, ")")) {
    // *tok = skip(tok, "int"); // 引数は必ずint
    var_delaration(tok, context);
    node->nparams++;
    if (check(tok, ",")) {
      *tok = skip(tok, ",");
    }
  }

  // 関数の本体をパース
  if (!check(tok, "{")) {
    error_at((*tok)->str, "'{'ではありません．");
  }
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
  if (equal(tok, "if")) {
    struct Node *cond, *stmt1, *stmt2;
    *tok = skip(tok, "(");
    cond = expr(tok, context);
    *tok = skip(tok, ")");
    stmt1 = stmt(tok, context);
    if (equal(tok, "else")) {
      stmt2 = stmt(tok, context);
    } else {
      stmt2 = NULL;
    }
    return new_node_if(cond, stmt1, stmt2);
  }
  if (equal(tok, "while")) {
    struct Node *cond, *stmt1;
    *tok = skip(tok, "(");
    cond = expr(tok, context);
    *tok = skip(tok, ")");
    stmt1 = stmt(tok, context);
    return new_node_while(cond, stmt1);
  }
  if (equal(tok, "for")) {
    context_push(context);
    struct Node *for_begin, *for_after, *cond, *stmt1;
    *tok = skip(tok, "(");
    if (equal(tok, ";")) {
      for_begin = NULL;
    } else {
      for_begin = expr(tok, context);
      *tok = skip(tok, ";");
    }
    if (equal(tok, ";")) {
      cond = NULL;
    } else {
      cond = expr(tok, context);
      *tok = skip(tok, ";");
    }
    if (equal(tok, ")")) {
      for_after = NULL;
    } else {
      for_after = expr(tok, context);
      *tok = skip(tok, ")");
    }
    stmt1 = stmt(tok, context);
    context_pop(context);
    return new_node_for(for_begin, cond, for_after, stmt1);
  }
  if (equal(tok, "return")) {
    struct Node *node = expr(tok, context);
    *tok = skip(tok, ";");
    return new_node_unary(ND_RETURN, node);
  }
  if (equal(tok, "{")) {
    struct Node head;
    head.rhs = NULL;
    struct Node *cur = &head;
    context_push(context);
    while (!equal(tok, "}")) {
      cur = cur->rhs = new_node_unary(ND_BLOCK, stmt(tok, context));
    }
    context_pop(context);
    return head.rhs;
  }
  if (check(tok, "int")) {
    return var_delaration(tok, context);
  }
  if (equal(tok, ";")) {
    return new_node(ND_EMPTY);
  }
  struct Node *node = expr(tok, context);
  *tok = skip(tok, ";");
  return node;
}

// expr = assign ( "==" assign | "&&" assign )*
struct Node *expr(struct Token **tok, struct Context **context) {
  struct Node *node = assign(tok, context);
  for (;;) {
    if (equal(tok, "||")) {
      node = new_node_binary(ND_OR, node, assign(tok, context));
    } else if (equal(tok, "&&")) {
      node = new_node_binary(ND_AND, node, assign(tok, context));
    } else {
      return node;
    }
  }
}

// assign  = equality ( "=" assign )?
struct Node *assign(struct Token **tok, struct Context **context) {
  struct Node *node = equality(tok, context);
  if (equal(tok, "=")) {
    node = new_node_binary(ND_ASSIGN, node, assign(tok, context));
  }
  return node;
}

// equality = relational ( "==" relational | "!=" relational )*
struct Node *equality(struct Token **tok, struct Context **context) {
  struct Node *node = relational(tok, context);

  for (;;) {
    if (equal(tok, "==")) {
      node = new_node_binary(ND_EQ, node, relational(tok, context));
    } else if (equal(tok, "!=")) {
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
    if (equal(tok, "<")) {
      node = new_node_binary(ND_LT, node, add(tok, context));
    } else if (equal(tok, "<=")) {
      node = new_node_binary(ND_LE, node, add(tok, context));
    } else if (equal(tok, ">")) {
      node = new_node_binary(ND_LT, add(tok, context), node);
    } else if (equal(tok, ">=")) {
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
    if (equal(tok, "+")) {
      node = new_node_binary(ND_ADD, node, mul(tok, context));
    } else if (equal(tok, "-")) {
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
    if (equal(tok, "*")) {
      node = new_node_binary(ND_MUL, node, unary(tok, context));
    } else if (equal(tok, "/")) {
      node = new_node_binary(ND_DIV, node, unary(tok, context));
    } else if (equal(tok, "%")) {
      node = new_node_binary(ND_MOD, node, unary(tok, context));
    } else {
      return node;
    }
  }
}

// unary = ( "+" | "-" )? primary
//       | "&" unary
//       | "*" unary
struct Node *unary(struct Token **tok, struct Context **context) {
  if (equal(tok, "+")) {
    return primary(tok, context);
  } else if (equal(tok, "-")) {
    return new_node_binary(ND_SUB, new_node_num(0, new_type(INT, NULL)), primary(tok, context));
  } else if (equal(tok, "&")) {
    return new_node_unary(ND_ADDR, unary(tok, context));
  } else if (equal(tok, "*")) {
    return new_node_unary(ND_DEREF, unary(tok, context));
  } else {
    return primary(tok, context);
  }
}

// primary = num
//         | ident ( "(" ( expr ( "," expr )* )? ")" )
//         | "(" expr ")"
struct Node *primary(struct Token **tok, struct Context **context) {
  if (equal(tok, "(")) {
    struct Node *node = expr(tok, context);
    *tok = skip(tok, ")");
    return node;
  } else if ((*tok)->kind == TK_IDENT) {
    if (equal(&(*tok)->next, "(")) { // 関数呼び出しのパース
      // 関数名をパース
      struct Node *node = new_node_binary(ND_FUNC_CALL, NULL, NULL);
      node->func_name = calloc((*tok)->len + 1, sizeof(char));
      memcpy(node->func_name, (*tok)->str, (*tok)->len * sizeof(char));
      consume(tok, TK_IDENT);

      // 引数をパース
      struct Node head;
      head.rhs = NULL;
      struct Node *cur = &head;
      while (!equal(tok, ")")) {
        cur = cur->rhs = new_node_binary(ND_FUNC_ARG, expr(tok, context), NULL);
        equal(tok, ",");
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
  } else if ((*tok)->kind == TK_NUM) {
    struct Node *node = new_node_num((*tok)->val, new_type(INT, NULL));
    consume(tok, TK_NUM);
    return node;
  }

  error_at((*tok)->str, "予期しないトークンです: %s", (*tok)->str);
  return NULL; // unreachable
}

struct Node *var_delaration(struct Token **tok, struct Context **context) {
  *tok = skip(tok, "int");
  struct Type *type = calloc(1, sizeof(struct Type));
  type->kind = INT;
  type->ptr_to = NULL;
  while (equal(tok, "*")) {
    struct Type *t = calloc(1, sizeof(struct Type));
    t->kind = PTR;
    t->ptr_to = type;
    type = t;
  }
  new_lvar(tok, context, type);
  consume(tok, TK_IDENT);
  return new_node(ND_VAR_DEF);
}

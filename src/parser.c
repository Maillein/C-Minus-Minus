#include "cmm.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Variable *new_variable(struct Type *type) {
  assert(type->kind != TY_FUNCTION && "関数型が与えられました");
  struct Variable *lvar = calloc(1, sizeof(struct Variable));
  lvar->prev = lvar;
  lvar->next = lvar;
  lvar->type = type;
  lvar->len = type->name->len;
  lvar->name = calloc(lvar->len + 1, sizeof(char));
  memcpy(lvar->name, type->name->str, lvar->len);
  return lvar;
}

struct VarList *new_varlist() {
  struct VarList *list = calloc(1, sizeof(struct VarList));
  list->head = calloc(1, sizeof(struct Variable));
  list->head->prev = list->head;
  list->head->next = list->head;
  list->len = 0;
  return list;
}

// 末尾に新しいVariableを追加
struct Variable *append_varlist(struct VarList *list, struct Type *type) {
  struct Variable *var = new_variable(type);
  var->next = list->head;
  var->prev = list->head->prev;
  var->next->prev = var;
  var->prev->next = var;
  list->len++;
  return var;
}

struct Variable *append_global(struct Type *type, struct Context **context) {
  return append_varlist(*(*context)->globals, type);
}

struct Variable *append_local(struct Type *type, struct Context **context) {
  struct Variable *v = append_varlist((*context)->locals, type);
  v->offset = (*context)->locals_offset;
  v->offset += size_of_type(get_base_type(v->type));
  (*context)->locals_offset += type->size;

  if ((*context)->max_local_offset < (*context)->locals_offset) {
    (*context)->max_local_offset = (*context)->locals_offset;
  }
  return v;
}

struct Variable *find_varlist(struct VarList *list, struct Token *tok) {
  for (struct Variable *v = list->head->next; v != list->head; v = v->next) {
    if (v->len == tok->len && !memcmp(v->name, tok->str, tok->len)) {
      return v;
    }
  }
  return NULL;
}

struct Variable *find_variable(struct Token **tok, struct Context **context) {
  for (struct Context *cont = *context; cont; cont = cont->next) {
    struct Variable *v = find_varlist(cont->locals, *tok);
    if (v) {
      return v;
    }
  }
  return find_varlist(*(*context)->globals, *tok);
}

struct Function *new_function(struct Type *type) {
  assert(type->kind == TY_FUNCTION && "関数型以外が与えられました");
  struct Function *func = calloc(1, sizeof(struct Function));
  func->prev = func;
  func->next = func;
  func->return_type = type->return_type;
  func->params = type->param;
  func->len = type->name->len;
  func->name = calloc(func->len + 1, sizeof(char));
  memcpy(func->name, type->name->str, func->len);
  return func;
}

struct FuncList *new_funclist() {
  struct FuncList *list = calloc(1, sizeof(struct FuncList));
  list->head = calloc(1, sizeof(struct Function));
  list->head->prev = list->head;
  list->head->next = list->head;
  list->len = 0;
  return list;
}

// 末尾に新しいFunctionを追加
struct Function *append_funclist(struct FuncList *list, struct Type *type) {
  struct Function *func = new_function(type);
  func->next = list->head;
  func->prev = list->head->prev;
  func->next->prev = func;
  func->prev->next = func;
  list->len++;
  return func;
}
struct Function *append_function(struct Type *type, struct Context **context) {
  return append_funclist(*(*context)->functions, type);
}

struct Function *find_funclist(struct FuncList *list, struct Token *tok) {
  for (struct Function *v = list->head->next; v != list->head; v = v->next) {
    if (v->len == tok->len && !memcmp(v->name, tok->str, tok->len)) {

      return v;
    }
  }
  return NULL;
}

struct Function *find_function(struct Token **tok, struct Context **context) {
  return find_funclist(*(*context)->functions, *tok);
}

void context_push(struct Context **head) {
  struct Context *context = calloc(1, sizeof(struct Context));
  context->next = *head;
  context->locals_offset = (*head)->locals_offset;
  context->max_local_offset = (*head)->max_local_offset;
  context->locals = new_varlist();
  context->globals = (*head)->globals;
  context->functions = (*head)->functions;
  *head = context;
}
void context_pop(struct Context **head) {
  if ((*head)->next->max_local_offset < (*head)->max_local_offset) {
    (*head)->next->max_local_offset = (*head)->max_local_offset;
  }
  *head = (*head)->next;
}

// <function-definition> ::= <type-specifier> <declarator> {<declaration>}* <compound-statement>
// <declaration> ::= <type-specifier> {<declarator> {= <expr>}?}* ;
// <type-specifier> ::= int
// <init-declarator> ::=
// <declarator> ::= <pointer>* <identifier> {<post-declarator}*
// <post-declarator> ::= [ <number>? ] <post-declarator>
//                     | ( <parameter-type-list>? )

struct Type *type_specifier(struct Token **tok);
struct Type *post_declarator(struct Token **tok, struct Type *ty);
struct Type *declarator(struct Token **tok, struct Type *base);

struct Type *type_specifier(struct Token **tok) {
  if (consume(tok, TK_INT)) {
    return primitive_type(TY_INT);
  }
  error_at((*tok)->str, "型ではありません: %s", TokenKind2str((*tok)->kind));
  return NULL;
}

struct Type *post_declarator(struct Token **tok, struct Type *ty) {
  if (consume(tok, TK_L_SBRACKET)) {
    int len = 0;
    if ((*tok)->kind == TK_INTEGER) {
      len = (*tok)->val;
      expect(tok, TK_INTEGER);
    }
    expect(tok, TK_R_SBRACKET);
    struct Type *pty = post_declarator(tok, ty);
    return array_of(pty, len);
  }

  if (consume(tok, TK_L_PAREN)) {
    struct Type *param = NULL;
    struct Type **cur = &param;
    while (!consume(tok, TK_R_PAREN)) {
      struct Type *spec = type_specifier(tok);
      *cur = declarator(tok, spec);
      cur = &(*cur)->next_param;
      consume(tok, TK_COMMA);
    }
    ty = function_type(ty);
    ty->param = param;
    return ty;
  }

  return ty;
}

struct Type *declarator(struct Token **tok, struct Type *base) {
  struct Type *type = base;
  while (consume(tok, TK_ASTER)) {
    type = pointer_to(type);
  }

  struct Token *name = *tok;
  expect(tok, TK_IDENT);

  type = post_declarator(tok, type);
  type->name = name;

  return type;
}

bool is_declaration(struct Token *tok) {
  for (; tok->kind != TK_SEMICOLON; tok = tok->next) {
    if (tok->kind == TK_L_CBRACKET) {
      return false;
    }
  }
  return true;
}

struct ASTNode *declaration(struct Token **tok, struct Context **context,
                            bool is_global);
struct ASTNode *func_definition(struct Token **tok, struct Context **context);
struct ASTNode *stmt(struct Token **tok, struct Context **context);
struct ASTNode *expr(struct Token **tok, struct Context **context);
struct ASTNode *assign(struct Token **tok, struct Context **context);
struct ASTNode *equality(struct Token **tok, struct Context **context);
struct ASTNode *relational(struct Token **tok, struct Context **context);
struct ASTNode *add(struct Token **tok, struct Context **context);
struct ASTNode *mul(struct Token **tok, struct Context **context);
struct ASTNode *unary(struct Token **tok, struct Context **context);
struct ASTNode *primary(struct Token **tok, struct Context **context);

struct ASTNode *declaration(struct Token **tok, struct Context **context,
                            bool is_global) {
  struct Type *base = type_specifier(tok);
  struct ASTNode *nhead = new_node(ND_VAR_DEF);
  struct ASTNode **cnode = &nhead;
  while (!consume(tok, TK_SEMICOLON)) {
    struct Type *ty = declarator(tok, base);
    struct Variable *lv = NULL;
    if (ty->kind == TY_FUNCTION) {
      append_function(ty, context);
    } else {
      if (is_global) {
        lv = append_global(ty, context);
      } else {
        lv = append_local(ty, context);
      }
    }

    if (consume(tok, TK_EQUAL)) {
      if (*cnode == NULL) {
        *cnode = new_node(ND_VAR_DEF);
      }
      struct ASTNode *lhs = new_node_var(lv, ty);
      struct ASTNode *rhs = assign(tok, context);
      (*cnode)->lhs = new_node_binary(ND_ASSIGN, lhs, rhs);
      cnode = &(*cnode)->rhs;
    }

    consume(tok, TK_COMMA);
  }
  return nhead;
}

// func_definition = "int" ident "(" ( "int" ident ( "," "int" ident )* )? ")" ( ";" | "{" stmt* "}" )
struct ASTNode *func_definition(struct Token **tok, struct Context **context) {
  struct Type *base_ty = type_specifier(tok);
  struct Type *func_ty = declarator(tok, base_ty);
  struct Function *func = append_function(func_ty, context);
  context_push(context);

  struct ASTNode *node = new_node(ND_FUNC_DEF);
  node->func = func;
  for (struct Type *ty = func->params; ty; ty = ty->next_param) {
    ty->size = size_of_type(ty);
    node->stack_size += ty->size;
    node->nparams++;

    struct Variable *v = append_local(ty, context);
    if (node->nparams == 1) {
      node->params = v;
    }
  }

  // 関数の本体をパース
  node->rhs = stmt(tok, context);

  context_pop(context);
  node->stack_size += (*context)->max_local_offset;
  // アラインメント
  node->stack_size = ((node->stack_size + 16 - 1) / 16) * 16;
  return node;
}

// stmt = expr? ";"
//      | "{" stmt* "}"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ( "else" stmt )?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | var_delaration ";"
struct ASTNode *stmt(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_IF)) {
    struct ASTNode *cond, *stmt1, *stmt2;
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
    struct ASTNode *cond, *stmt1;
    expect(tok, TK_L_PAREN);
    cond = expr(tok, context);
    expect(tok, TK_R_PAREN);
    stmt1 = stmt(tok, context);
    return new_node_while(cond, stmt1);
  }

  if (consume(tok, TK_FOR)) {
    context_push(context);
    struct ASTNode *for_begin, *for_after, *cond, *stmt1;
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
    struct ASTNode *node = expr(tok, context);
    expect(tok, TK_SEMICOLON);
    return new_node_unary(ND_RETURN, node);
  }

  if (consume(tok, TK_L_CBRACKET)) {
    struct ASTNode head;
    head.rhs = NULL;
    struct ASTNode *cur = &head;
    context_push(context);
    while (!consume(tok, TK_R_CBRACKET)) {
      cur = cur->rhs = new_node_unary(ND_BLOCK, stmt(tok, context));
    }
    context_pop(context);
    return head.rhs;
  }

  if ((*tok)->kind == TK_INT) {
    struct ASTNode *node = declaration(tok, context, false);
    return node;
  }

  if (!consume(tok, TK_SEMICOLON)) {
    struct ASTNode *node = expr(tok, context);
    expect(tok, TK_SEMICOLON);
    return node;
  }

  while (consume(tok, TK_SEMICOLON))
    ;
  return new_node(ND_EMPTY); // unreachable
}

// expr = assign ( "==" assign | "&&" assign )*
struct ASTNode *expr(struct Token **tok, struct Context **context) {
  struct ASTNode *node = assign(tok, context);
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
struct ASTNode *assign(struct Token **tok, struct Context **context) {
  struct ASTNode *node = equality(tok, context);
  if (consume(tok, TK_EQUAL)) {
    node = new_node_binary(ND_ASSIGN, node, assign(tok, context));
  }
  return node;
}

// equality = relational ( "==" relational | "!=" relational )*
struct ASTNode *equality(struct Token **tok, struct Context **context) {
  struct ASTNode *node = relational(tok, context);

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
struct ASTNode *relational(struct Token **tok, struct Context **context) {
  struct ASTNode *node = add(tok, context);

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
struct ASTNode *add(struct Token **tok, struct Context **context) {
  struct ASTNode *node = mul(tok, context);

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
struct ASTNode *mul(struct Token **tok, struct Context **context) {
  struct ASTNode *node = unary(tok, context);

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
struct ASTNode *unary(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_PLUS)) {
    return primary(tok, context);
  } else if (consume(tok, TK_MINUS)) {
    return new_node_binary(ND_SUB, new_node_num(0, primitive_type(TY_INT)),
                           primary(tok, context));
  } else if (consume(tok, TK_AMP)) {
    return new_node_unary(ND_ADDR, unary(tok, context));
  } else if (consume(tok, TK_ASTER)) {
    return new_node_unary(ND_DEREF, unary(tok, context));
  } else if (consume(tok, TK_SIZEOF)) {
    struct ASTNode *node = primary(tok, context);
    solve_node_type(node);
    return new_node_num(node->type->size, primitive_type(TY_INT));
  } else {
    return primary(tok, context);
  }
}

// primary = num
//         | ident ( "(" ( expr ( "," expr )* )? ")" )
//         | "(" expr ")"
struct ASTNode *primary(struct Token **tok, struct Context **context) {
  if (consume(tok, TK_L_PAREN)) {
    struct ASTNode *node = expr(tok, context);
    expect(tok, TK_R_PAREN);
    return node;
  } else if ((*tok)->kind == TK_IDENT) {
    if ((*tok)->next->kind == TK_L_PAREN) { // 関数呼び出しのパース
      // 関数名をパース
      struct ASTNode *node = new_node_binary(ND_FUNC_CALL, NULL, NULL);
      node->func_name = calloc((*tok)->len + 1, sizeof(char));
      memcpy(node->func_name, (*tok)->str, (*tok)->len * sizeof(char));
      struct Function *func = find_function(tok, context);
      if (func == NULL) {
        error_at((*tok)->str, "未定義の関数です");
      } else {
        node->type = func->return_type;
      }
      expect(tok, TK_IDENT);
      expect(tok, TK_L_PAREN);

      // 引数をパース
      struct ASTNode head;
      head.rhs = NULL;
      struct ASTNode *cur = &head;
      while (!consume(tok, TK_R_PAREN)) {
        cur = cur->rhs = new_node_binary(ND_FUNC_ARG, expr(tok, context), NULL);
        consume(tok, TK_COMMA);
      }
      node->args = head.rhs;

      return node;
    } else { // 変数のパース
      struct Variable *lvar = find_variable(tok, context);
      if (lvar == NULL) {
        error_at((*tok)->str, "未定義の変数です");
        // lvar = new_lvar(tok, context);
      }
      consume(tok, TK_IDENT);
      return new_node_var(lvar, lvar->type);
    }
  } else if ((*tok)->kind == TK_INTEGER) {
    struct ASTNode *node = new_node_num((*tok)->val, primitive_type(TY_INT));
    expect(tok, TK_INTEGER);
    return node;
  }

  error_at((*tok)->str, "予期しないトークンです: %s",
           TokenKind2str((*tok)->kind));
  return NULL; // unreachable
}

// program = func_definition*
struct ASTNode *parse(struct Token **tok) {
  struct ASTNode *node = new_node(ND_PROGRAM);
  struct VarList *globals = new_varlist();
  struct FuncList *functions = new_funclist();
  struct Context *context = calloc(1, sizeof(struct Context));
  context->globals = &globals;
  context->functions = &functions;
  struct ASTNode *cur = node;
  while (!at_eof(tok)) {
    cur = cur->rhs = new_node(ND_PROGRAM);
    if (is_declaration(*tok)) {
      cur->lhs = declaration(tok, &context, true);
    } else {
      cur->lhs = func_definition(tok, &context);
    }
  }
  return node->rhs;
}

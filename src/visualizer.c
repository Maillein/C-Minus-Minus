#include "cmm.h"
#include <stdio.h>
#include <stdlib.h>

int node_id() {
  static int node_id = 1;
  return node_id++;
}
static FILE *fp;

char *nodekind_to_str(enum NodeKind kind) {
  char *ret;
  switch (kind) {
  case ND_PROGRAM:
    ret = "ND_PROGRAM";
    break;
  case ND_FUNC_DEF:
    ret = "ND_FUNC_DEF";
    break;
  case ND_BLOCK:
    ret = "ND_BLOCK";
    break;
  case ND_EMPTY:
    ret = "ND_EMPTY";
    break;
  case ND_IF:
    ret = "ND_IF";
    break;
  case ND_WHILE:
    ret = "ND_WHILE";
    break;
  case ND_FOR:
    ret = "ND_FOR";
    break;
  case ND_RETURN:
    ret = "ND_RETURN";
    break;
  case ND_VAR_DEF:
    ret = "ND_VAR_DEF";
    break;
  case ND_ADD:
    ret = "ND_ADD";
    break;
  case ND_SUB:
    ret = "ND_SUB";
    break;
  case ND_MUL:
    ret = "ND_MUL";
    break;
  case ND_DIV:
    ret = "ND_DIV";
    break;
  case ND_MOD:
    ret = "ND_MOD";
    break;
  case ND_EQ:
    ret = "ND_EQ";
    break;
  case ND_NE:
    ret = "ND_NE";
    break;
  case ND_LT:
    ret = "ND_LT";
    break;
  case ND_LE:
    ret = "ND_LE";
    break;
  case ND_OR:
    ret = "ND_OR";
    break;
  case ND_AND:
    ret = "ND_AND";
    break;
  case ND_ASSIGN:
    ret = "ND_ASSIGN";
    break;
  case ND_FUNC_CALL:
    ret = "ND_FUNC_CALL";
    break;
  case ND_NUM:
    ret = "ND_NUM";
    break;
  case ND_LVAR:
    ret = "ND_LVAR";
    break;
  case ND_FUNC_ARG:
    ret = "ND_FUNC_CALL";
    break;
  case ND_ADDR:
    ret = "ND_ADDR";
    break;
  case ND_DEREF:
    ret = "ND_DEREF";
    break;
  }
  return ret;
}

char *typekind_to_str(enum TypeKind kind) {
  char *ret;
  switch (kind) {
  case INT:
    ret = "INT";
    break;
  case PTR:
    ret = "PTR";
    break;
  }
  return ret;
}

void vis_func_def(struct Node *node);
void vis_stmt(struct Node *node, char *p_name);
void vis_expr(struct Node *node, char *p_name);

void vis_program(struct Node *node) {
  for (; node; node = node->rhs) {
    solve_node_type(node->lhs);
    vis_func_def(node->lhs);
  }
}

void vis_func_def(struct Node *node) {
  char name[32];
  if (!node) {
    return;
  }
  fprintf(fp, "  subgraph cluster_%s {\n", node->func_name);
  fprintf(fp, "    lable = \"%s\"\n", node->func_name);
  sprintf(name, "%s%d", nodekind_to_str(node->kind), node_id());
  fprintf(fp, "    %s[label = \"int %s()\", shape = \"box\"];\n", name,
          node->func_name);
  vis_stmt(node->rhs, name);
  fprintf(fp, "  }\n");
}

void vis_stmt(struct Node *node, char *p_name) {
  char name[32];
  if (!node) {
    return;
  }
  sprintf(name, "%s%d", nodekind_to_str(node->kind), node_id());
  fprintf(fp, "    %s[label = \"%s\", shape = \"box\"];\n", name,
          nodekind_to_str(node->kind));
  fprintf(fp, "    %s -> %s;\n", p_name, name);
  if (node->kind == ND_BLOCK) {
    for (; node; node = node->rhs) {
      vis_stmt(node->lhs, name);
    }
    return;
  }
  if (node->kind == ND_RETURN) {
    vis_expr(node->lhs, name);
    return;
  }
  if (node->kind == ND_IF) {
    vis_expr(node->cond, name);
    vis_stmt(node->stmt1, name);
    vis_stmt(node->stmt2, name);
    return;
  }
  if (node->kind == ND_WHILE) {
    vis_expr(node->cond, name);
    vis_stmt(node->stmt1, name);
    return;
  }
  if (node->kind == ND_FOR) {
    vis_expr(node->init, name);
    vis_expr(node->cond, name);
    vis_expr(node->update, name);
    vis_stmt(node->stmt1, name);
    return;
  }
  if (node->kind == ND_ASSIGN || node->kind == ND_FUNC_CALL) {
    vis_expr(node, name);
    return;
  }
}

void vis_expr(struct Node *node, char *p_name) {
  char name[32];
  if (!node) {
    return;
  }
  sprintf(name, "%s%d", nodekind_to_str(node->kind), node_id());
  if (node->kind == ND_NUM) {
    fprintf(fp, "    %s[label = \"%d [%s]\", shape = \"oval\"];\n", name,
            node->val, typekind_to_str(node->type->kind));
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    return;
  }
  if (node->kind == ND_LVAR) {
    fprintf(fp, "    %s[label = \"%s [%s]\", shape = \"oval\"];\n", name,
            node->lvar->name, typekind_to_str(node->type->kind));
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    return;
  }
  if (node->kind == ND_ADDR) {
    fprintf(fp, "    %s[label = \"%s\", shape = \"box\"];\n", name,
            nodekind_to_str(node->kind));
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    return;
  }
  if (node->kind == ND_DEREF) {
    fprintf(fp, "    %s[label = \"%s\", shape = \"box\"];\n", name,
            nodekind_to_str(node->kind));
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    vis_expr(node->lhs, name);
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    fprintf(fp, "    %s[label = \"%s %s()\", shape = \"oval\"];\n",
            name, typekind_to_str(node->type->kind), node->func_name);
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    return;
  }
  if (node->kind == ND_ASSIGN) {
    fprintf(fp, "    %s[label = \"%s\", shape = \"box\"];\n", name,
            nodekind_to_str(node->kind));
    fprintf(fp, "    %s -> %s;\n", p_name, name);
    vis_expr(node->lhs, name);
    vis_expr(node->rhs, name);
    return;
  }
  fprintf(fp, "    %s[label = \"%s [%s]\", shape = \"box\"];\n", name,
          nodekind_to_str(node->kind), typekind_to_str(node->type->kind));
  fprintf(fp, "    %s -> %s;\n", p_name, name);
  vis_expr(node->lhs, name);
  vis_expr(node->rhs, name);
  return;
}

void vis_ast(struct Node *node) {
  fp = fopen("AST.dot", "w");
  if (fp == NULL) {
    fprintf(stderr, "AST.dotを開けませんでした．\n");
    exit(1);
  }

  fprintf(fp, "digraph AST {\n");
  fprintf(fp, "  graph [\n"
              "    charset = \"UTF-8\",\n"
              "    fontsize = 18,\n"
              "  ];\n");
  fprintf(fp, "  node [\n"
              "    charset = \"UTF-8\",\n"
              "    fontsize = 18,\n"
              "  ];\n");
  fprintf(fp, "  edge [\n"
              "    fontsize = 18,\n"
              "    charset = \"UTF-8\",\n"
              "    fontsize = 18,\n"
              "  ];\n");
  vis_program(node);
  fprintf(fp, "}");

  fclose(fp);
  fp = NULL;
}

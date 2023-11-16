#include "cmm.h"
#include <stdio.h>

static int label_if;
static int label_while;
static int label_for;
static char *label_func_name;

void gen_lval(struct Node *node) {
  if (node->kind != ND_LVAR) {
    error("左辺値が変数ではありません");
  }

  printf("  lea rdi, [rbp-%d]\n", node->offset);
}

// 「文」を生成する．assign, returnを除き，RAXに値を残さない．
void gen_stmt(struct Node *node) {
  if (node->kind == ND_ASSIGN) {
    gen_expr(node);
    return;
  }
  if (node->kind == ND_RETURN) {
    gen_expr(node->lhs);
    printf("  jmp .L_%s_epilogue\n", label_func_name);
    return;
  }
  if (node->kind == ND_IF) {
    int local_label = label_if++;
    gen_expr(node->cond);
    // printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->stmt2 != NULL) {
      printf("  je .Lelse_%d\n", local_label);
      gen_stmt(node->stmt1);
      printf("  je  .Lend_if_%d\n", local_label);
      printf(".Lelse_%d:\n", local_label);
      gen_stmt(node->stmt2);
    } else {
      printf("  je .Lend_if_%d\n", local_label);
      gen_stmt(node->stmt1);
    }
    printf(".Lend_if_%d:\n", local_label);
    label_if++;
    return;
  }
  if (node->kind == ND_WHILE) {
    int local_label = label_while++;
    printf(".Lbegin_while_%d:\n", local_label);
    gen_expr(node->cond);
    // printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend_while_%d\n", local_label);
    gen_expr(node->stmt1);
    printf("  jmp .Lbegin_while_%d\n", local_label);
    printf(".Lend_while_%d:\n", local_label);
    return;
  }
  if (node->kind == ND_FOR) {
    int local_label = label_for++;
    if (node->for_begin) {
      gen_expr(node->for_begin);
    }

    printf(".Lbegin_for_%d:\n", local_label);
    if (node->cond) {
      gen_expr(node->cond);
      // printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_for_%d\n", local_label);
      gen_stmt(node->stmt1);
      if (node->for_after) {
        gen_expr(node->for_after);
      }
      printf("  jmp .Lbegin_for_%d\n", local_label);
      printf(".Lend_for_%d:\n", local_label);
    } else {
      gen_stmt(node->stmt1);
      if (node->for_after) {
        gen_expr(node->for_after);
      }
      printf("  jmp .Lbegin_for_%d\n", local_label);
    }
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    gen_expr(node);
    return;
  }
  if (node->kind == ND_BLOCK) {
    for (; node; node = node->rhs) {
      gen_stmt(node->lhs);
    }
    return;
  }
  if (node->kind == ND_EMPTY) {
    return;
  }
}

// 「式」を生成する．評価結果はRAXレジスタに格納される．
void gen_expr(struct Node *node) {
  if (node->kind == ND_NUM) {
    printf("  mov rax, %d\n", node->val);
    return;
  }
  if (node->kind == ND_LVAR) {
    gen_lval(node);
    printf("  mov rax, [rdi]\n");
    return;
  }
  if (node->kind == ND_ASSIGN) {
    gen_expr(node->rhs);
    gen_lval(node->lhs);
    printf("  mov [rdi], rax\n");
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    int nargs = 0;
    for (struct Node *arg = node->args; arg && nargs <= 6; arg = arg->rhs) {
      gen_expr(arg->lhs);
      printf("  push rax\n");
      nargs++;
    }
    for (; nargs; nargs--) {
      if (nargs == 1) {
        printf("  pop rdi\n");
      } else if (nargs == 2) {
        printf("  pop rsi\n");
      } else if (nargs == 3) {
        printf("  pop rdx\n");
      } else if (nargs == 4) {
        printf("  pop rcx\n");
      } else if (nargs == 5) {
        printf("  pop r8\n");
      } else if (nargs == 6) {
        printf("  pop r9\n");
      }
    }
    printf("  call %s\n", node->func_name);
    return;
  }

  gen_expr(node->lhs);
  printf("  push rax\n");
  gen_expr(node->rhs);
  printf("  push rax\n");

  printf("  pop rdi\n");
  printf("  pop rax\n");

  if (node->kind == ND_ADD) {
    printf("  add rax, rdi\n");
  } else if (node->kind == ND_SUB) {
    printf("  sub rax, rdi\n");
  } else if (node->kind == ND_MUL) {
    printf("  imul rax, rdi\n");
  } else if (node->kind == ND_DIV) {
    printf("  cqo\n");
    printf("  idiv rax, rdi\n");
  } else if (node->kind == ND_EQ) {
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
  } else if (node->kind == ND_NE) {
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
  } else if (node->kind == ND_LT) {
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
  } else if (node->kind == ND_LE) {
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
  }
}


void codegen(struct Node *node) {
  if (node->kind != ND_FUNC_DEF) {
    error("関数定義ではありません");
  }
  label_func_name = node->func_name;
  printf(".global %s\n", label_func_name);
  printf("%s:\n", label_func_name);

  // プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 0x100\n"); // ローカル変数の領域(16byte境界でアライメント)

  gen_stmt(node->rhs);

  // エピローグ
  printf(".L_%s_epilogue:\n", label_func_name);
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

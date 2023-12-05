#include "cmm.h"
#include <stdio.h>

static int label_if;
static int label_while;
static int label_for;
static int label_or;
static int label_and;
static char *label_func_name;

void gen_stmt(struct Node *node);
void gen_expr(struct Node *node);

// アドレスをrdiレジスタに生成しているのはまずいかも?
void gen_lval(struct Node *node, char *reg) {
  if (node->kind == ND_LVAR) {
    printf("  lea %s, [rbp-%d]\n", reg, node->lvar->offset);
    return;
  }
  if (node->kind == ND_DEREF) {
    printf("  push rax\n");
    gen_expr(node->lhs);
    printf("  mov %s, rax\n", reg);
    printf("  pop rax\n");
    return;
  }
  error("左辺値が変数ではありません");
}

// 「文」を生成する．assign, returnを除き，RAXに値を残さない．
void gen_stmt(struct Node *node) {
  if (node->kind == ND_FUNC_DEF) {
    if (node->rhs == NULL) { // 関数プロトタイプ宣言
      return;
    }
    label_func_name = node->func_name;
    printf(".global %s\n", label_func_name);
    printf("%s:\n", label_func_name);

    // プロローグ
    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    node->stack_size += node->stack_size % 16; // ローカル変数のアライメント
    printf("  sub rsp, %d\n",
           node->stack_size); // ローカル変数の領域(16byte境界でアライメント)
    for (int i = 1; i <= node->nparams; i++) {
      if (i == 1) {
        printf("  mov [rbp-8], rdi\n");
      } else if (i == 2) {
        printf("  mov [rbp-16], rsi\n");
      } else if (i == 3) {
        printf("  mov [rbp-24], rdx\n");
      } else if (i == 4) {
        printf("  mov [rbp-32], rcx\n");
      } else if (i == 5) {
        printf("  mov [rbp-40], r8\n");
      } else if (i == 6) {
        printf("  mov [rbp-48], r9\n");
      }
    }

    gen_stmt(node->rhs);

    // エピローグ
    printf(".L_%s_epilogue:\n", label_func_name);
    printf("  leave\n");
    printf("  ret\n");
    return;
  }
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
    printf("# begin if cond (%d)\n", local_label);
    gen_expr(node->cond);
    printf("# end if cond (%d)\n", local_label);
    printf("  cmp rax, 0\n");
    if (node->stmt2 != NULL) {
      printf("  je .Lelse_%d\n", local_label);
      printf("# begin if stmt1 (%d)\n", local_label);
      gen_stmt(node->stmt1);
      printf("# end if stmt1 (%d)\n", local_label);
      printf("  jmp  .Lend_if_%d\n", local_label);
      printf(".Lelse_%d:\n", local_label);
      printf("# end if stmt2 (%d)\n", local_label);
      gen_stmt(node->stmt2);
      printf("# end if stmt2 (%d)\n", local_label);
    } else {
      printf("  # begin if stmt1 (%d)\n", local_label);
      printf("  je .Lend_if_%d\n", local_label);
      printf("# end if stmt1 (%d)\n", local_label);
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
    if (node->init) {
      printf("# begin for init\n");
      gen_expr(node->init);
      printf("# end for init\n");
    }

    printf(".Lbegin_for_%d:\n", local_label);
    if (node->cond) {
      printf("# begin for cond\n");
      gen_expr(node->cond);
      // printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_for_%d\n", local_label);
      printf("  # end for cond\n");
      printf("  # begin for stmt\n");
      gen_stmt(node->stmt1);
      printf("# end for stmt\n");
      if (node->update) {
        printf("# begin for update\n");
        gen_expr(node->update);
        printf("# end for update\n");
      }
      printf("  jmp .Lbegin_for_%d\n", local_label);
      printf(".Lend_for_%d:\n", local_label);
    } else {
      gen_stmt(node->stmt1);
      if (node->update) {
        gen_expr(node->update);
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
    // fprintf(stderr, "# node->lvar->name: %s\n", node->lvar->name);
    // fprintf(stderr, "# node->type->ty: ");
    // for (struct Type *t = node->->type; t != NULL; t = t->ptr_to) {
    //   if (t->ty == INT) {
    //     fprintf(stderr, "INT");
    //   } else {
    //     fprintf(stderr, "PTR -> ");
    //   }
    // }
    // fprintf(stderr, "\n");
    gen_lval(node, "rdi");
    printf("  mov rax, [rdi]\n");
    return;
  }
  if (node->kind == ND_ADDR) {
    gen_lval(node->lhs, "rax");
    return;
  }
  if (node->kind == ND_DEREF) {
    gen_expr(node->lhs);
    printf("  mov rax, [rax]\n");
    return;
  }
  if (node->kind == ND_ASSIGN) {
    gen_expr(node->rhs);
    gen_lval(node->lhs, "rdi");
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
  if (node->kind == ND_OR) {
    int local_label = label_or++;
    gen_expr(node->lhs);
    printf("  cmp rax, 0\n");
    printf("  jne .L_or_true_%d\n", local_label); // 左辺が真だったら，抜ける
    gen_expr(node->rhs);
    printf("  cmp rax, 0\n");
    printf("  jne .L_or_true_%d\n", local_label); // 右辺が真だったら，抜ける
    printf("  mov rax, 0\n");
    printf("  jmp .L_or_end_%d\n", local_label);
    printf(".L_or_true_%d:\n", local_label);
    printf("  mov rax, 1\n");
    printf(".L_or_end_%d:\n", local_label);
    return;
  }
  if (node->kind == ND_AND) {
    int local_label = label_and++;
    gen_expr(node->lhs);
    printf("  cmp rax, 0\n");
    printf("  je .L_and_false_%d\n", local_label); // 左辺が偽だったら，抜ける
    gen_expr(node->rhs);
    printf("  cmp rax, 0\n");
    printf("  je .L_and_false_%d\n", local_label); // 右辺が真だったら，抜ける
    printf("  mov rax, 1\n");
    printf("  jmp .L_and_end_%d\n", local_label);
    printf(".L_and_false_%d:\n", local_label);
    printf("  mov rax, 0\n");
    printf(".L_and_end_%d:\n", local_label);
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
  } else if (node->kind == ND_MOD) {
    printf("  cqo\n");
    printf("  idiv rax, rdi\n");
    printf("  mov  rax, rdx\n");
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
  for (; node; node = node->rhs) {
    gen_stmt(node->lhs);
  }
}

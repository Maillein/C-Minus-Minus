#include "cmm.h"
#include <stdio.h>

static int label_if;
static int label_while;
static int label_for;

void gen_lval(struct Node *node) {
  if (node->kind != ND_LVAR) {
    error("左辺値が変数ではありません");
  }

  printf("  mov rax, rbp # 左辺値アドレス生成開始\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax # 左辺値アドレス生成終了\n");
}

void gen(struct Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d # 即値\n", node->val);
    return;
  }
  if (node->kind == ND_LVAR) {
    gen_lval(node);
    printf("  pop rax # 左辺値アドレス読み込み\n");
    printf("  mov rax, [rax] # 左辺値読み込み\n");
    printf("  push rax # 左辺値読み込み完了\n");
    return;
  }
  if (node->kind == ND_ASSIGN) {
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  }
  if (node->kind == ND_BLOCK) {
    for (; node; node = node->rhs) {
      gen(node->lhs);
      // ステートメントの結果をポップ
      printf("  pop rax\n");
    }
    return;
  }
  if (node->kind == ND_RETURN) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  }
  if (node->kind == ND_IF) {
    int local_label = label_if++;
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    if (node->stmt2 != NULL) {
      printf("  je .Lelse_%d\n", local_label);
      gen(node->stmt1);
      printf("  je  .Lend_if_%d\n", local_label);
      printf(".Lelse_%d:\n", local_label);
      gen(node->stmt2);
    } else {
      printf("  je .Lend_if_%d\n", local_label);
      gen(node->stmt1);
    }
    printf(".Lend_if_%d:\n", local_label);
    label_if++;
    return;
  }
  if (node->kind == ND_WHILE) {
    int local_label = label_while++;
    printf(".Lbegin_while_%d:\n", local_label);
    gen(node->cond);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend_while_%d\n", local_label);
    gen(node->stmt1);
    printf("  jmp .Lbegin_while_%d\n", local_label);
    printf(".Lend_while_%d:\n", local_label);
    return;
  }
  if (node->kind == ND_FOR) {
    int local_label = label_for++;
    if (node->for_begin) {
      gen(node->for_begin);
    }

    printf(".Lbegin_for_%d:\n", local_label);
    if (node->cond) {
      gen(node->cond);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend_for_%d\n", local_label);
      gen(node->stmt1);
      if (node->for_after) {
        gen(node->for_after);
      }
      printf("  jmp .Lbegin_for_%d\n", local_label);
      printf(".Lend_for_%d:\n", local_label);
    } else {
      gen(node->stmt1);
      if (node->for_after) {
        gen(node->for_after);
      }
      printf("  jmp .Lbegin_for_%d\n", local_label);
    }
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

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
  printf("  push rax\n");
}

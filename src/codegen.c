#include "cmm.h"
#include <assert.h>
#include <stdio.h>

static int label_if;
static int label_while;
static int label_for;
static int label_or;
static int label_and;
static char *label_func_name;

static char *RAX[4] = {"al", "ax", "eax", "rax"};
static char *RBX[4] = {"bl", "bx", "ebx", "rbx"};
static char *RCX[4] = {"cl", "dx", "ecx", "rcx"};
static char *RDX[4] = {"dl", "dx", "edx", "rdx"};
static char *RDI[4] = {"dil", "di", "edi", "rdi"};
static char *RSI[4] = {"sil", "si", "esi", "rsi"};
static char *R8[4] = {"r8b", "r8w", "r8d", "r8"};
static char *R9[4] = {"r9b", "r9w", "r9d", "r9"};
static char **FUNC_REG[6] = {RDI, RSI, RDX, RCX, R8, R9};

void gen_stmt(struct Node *node);
void gen_expr(struct Node *node);

void move_imm_to_register(char **dst, long imm, struct Type *type) {
  switch (type->kind) {
  case INT:
    printf("  mov %s, %ld\n", dst[2], imm);
    break;
  case PTR:
    assert(0);
    break;
  }
}

void move_register_to_memory(char *dst[4], char *src[4], struct Type *type) {
  switch (type->kind) {
  case INT:
    printf("  mov DWORD PTR [%s], %s\n", dst[3], src[2]);
    break;
  case PTR:
    printf("  mov QWORD PTR [%s], %s\n", dst[3], src[3]);
    break;
  }
}

void move_memory_to_register(char *dst[4], char *src[4], struct Type *type) {
  switch (type->kind) {
  case INT:
    printf("  mov %s, DWORD PTR [%s]\n", dst[2], src[3]);
    break;
  case PTR:
    printf("  mov %s, QWORD PTR [%s]\n", dst[3], src[3]);
    break;
  }
}

void move_register_to_register(char *dst[4], char *src[4], struct Type *type) {
  switch (type->kind) {
  case INT:
    printf("  mov %s, %s\n", dst[2], src[2]);
    break;
  case PTR:
    printf("  mov %s, %s\n", dst[3], src[3]);
    break;
  }
}

void push_imm(long imm) { printf("  push %ld\n", imm); }

void push_register(char *src[4]) { printf("  push %s\n", src[3]); }

void pop_register(char *dst[4]) { printf("  pop %s\n", dst[3]); }

void pop_memory(char *dst[4]) { printf("  pop [%s]\n", dst[3]); }

void compare_register_and_imm(char *dst[4], long imm, struct Type *type) {
  switch (type->kind) {
  case INT:
    printf("  cmp  %s, %ld\n", dst[3], imm);
    break;
  case PTR:
    printf("  cmp  %s, %ld\n", dst[4], imm);
    break;
  }
}

// アドレスをrdiレジスタに生成しているのはまずいかも?
void gen_lval(struct Node *node, char *reg[4]) {
  if (node->kind == ND_LVAR) {
    printf("  lea %s, [rbp-%d]\n", reg[3], node->lvar->offset);
    return;
  }
  if (node->kind == ND_DEREF) {
    push_register(RAX);
    gen_expr(node->lhs);
    printf("  mov %s, rax\n", reg[3]);
    pop_register(RAX);
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
    if (node->stack_size % 16 != 0) {
      node->stack_size += 16 - node->stack_size % 16; // ローカル変数のアライメント
    }
    printf("  sub rsp, %d\n",
           node->stack_size); // ローカル変数の領域(16byte境界でアライメント)
    int nparams = node->nparams - 1;
    // total_offsetの計算開始
    int total_offset = 0;
    for (struct LVar *param = node->params; nparams >= 0; param = param->next) {
      switch (param->type->kind) {
      case INT:
        total_offset += 4;
        break;
      case PTR:
        total_offset += 8;
        break;
      }
      nparams--;
    }
    // total_offsetの計算終了

    nparams = node->nparams - 1;
    for (struct LVar *param = node->params; nparams >= 0; param = param->next) {
      switch (param->type->kind) {
      case INT:
        printf("  mov [rbp-%d], %s # %s\n", total_offset, FUNC_REG[nparams][2], param->name);
        total_offset -= 4;
        break;
      case PTR:
        printf("  mov [rbp-%d], %s # %s\n", total_offset, FUNC_REG[nparams][3], param->name);
        total_offset -= 8;
        break;
      }
      nparams--;
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
    gen_expr(node->cond);
    compare_register_and_imm(RAX, 0, node->cond->type);
    if (node->stmt2 != NULL) {
      printf("  je .Lelse_%d\n", local_label);
      gen_stmt(node->stmt1);
      printf("  jmp  .Lend_if_%d\n", local_label);
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
    compare_register_and_imm(RAX, 0, node->cond->type);
    printf("  je .Lend_while_%d\n", local_label);
    gen_expr(node->stmt1);
    printf("  jmp .Lbegin_while_%d\n", local_label);
    printf(".Lend_while_%d:\n", local_label);
    return;
  }
  if (node->kind == ND_FOR) {
    int local_label = label_for++;
    if (node->init) {
      gen_expr(node->init);
    }

    printf(".Lbegin_for_%d:\n", local_label);
    if (node->cond) {
      gen_expr(node->cond);
      compare_register_and_imm(RAX, 0, node->cond->type);
      printf("  je .Lend_for_%d\n", local_label);
      gen_stmt(node->stmt1);
      if (node->update) {
        gen_expr(node->update);
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
    move_imm_to_register(RAX, node->val, node->type);
    return;
  }
  if (node->kind == ND_LVAR) {
    gen_lval(node, RDI);
    move_memory_to_register(RAX, RDI, node->type);
    return;
  }
  if (node->kind == ND_ADDR) {
    gen_lval(node->lhs, RAX);
    return;
  }
  if (node->kind == ND_DEREF) {
    gen_expr(node->lhs);
    printf("  mov rax, [rax]\n");
    return;
  }
  if (node->kind == ND_ASSIGN) {
    gen_expr(node->rhs);
    gen_lval(node->lhs, RDI);
    move_register_to_memory(RDI, RAX, node->rhs->type);
    return;
  }
  if (node->kind == ND_FUNC_CALL) {
    int nargs = 0;
    for (struct Node *arg = node->args; arg && nargs <= 6; arg = arg->rhs) {
      gen_expr(arg->lhs);
      push_register(RAX);
      nargs++;
    }
    for (; nargs; nargs--) {
      pop_register(FUNC_REG[nargs - 1]);
    }
    printf("  call %s\n", node->func_name);
    return;
  }
  if (node->kind == ND_OR) {
    int local_label = label_or++;
    gen_expr(node->lhs);
    compare_register_and_imm(RAX, 0, node->lhs->type);
    printf("  jne .L_or_true_%d\n", local_label); // 左辺が真だったら，抜ける
    gen_expr(node->rhs);
    compare_register_and_imm(RAX, 0, node->rhs->type);
    printf("  jne .L_or_true_%d\n", local_label); // 右辺が真だったら，抜ける
    printf("  mov rax, 0\n");
    move_imm_to_register(RAX, 0, node->type);
    printf("  jmp .L_or_end_%d\n", local_label);
    printf(".L_or_true_%d:\n", local_label);
    move_imm_to_register(RAX, 1, node->type);
    printf(".L_or_end_%d:\n", local_label);
    return;
  }
  if (node->kind == ND_AND) {
    int local_label = label_and++;
    gen_expr(node->lhs);
    compare_register_and_imm(RAX, 0, node->lhs->type);
    printf("  je .L_and_false_%d\n", local_label); // 左辺が偽だったら，抜ける
    gen_expr(node->rhs);
    compare_register_and_imm(RAX, 0, node->rhs->type);
    printf("  je .L_and_false_%d\n", local_label); // 右辺が真だったら，抜ける
    move_imm_to_register(RAX, 1, node->type);
    printf("  jmp .L_and_end_%d\n", local_label);
    printf(".L_and_false_%d:\n", local_label);
    move_imm_to_register(RAX, 0, node->type);
    printf(".L_and_end_%d:\n", local_label);
    return;
  }

  gen_expr(node->lhs);
  push_register(RAX);
  gen_expr(node->rhs);
  push_register(RAX);

  pop_register(RDI);
  pop_register(RAX);

  if (node->kind == ND_ADD) {
    switch (node->type->kind) {
    case INT:
      printf("  add %s, %s\n", RAX[2], RDI[2]);
      break;
    case PTR:
      if (node->rhs->type->kind != PTR) {
        printf("  imul %s, %d\n", RDI[3], node->lhs->type->ptr_to->size);
      }
      printf("  add %s, %s\n", RAX[3], RDI[3]);
      break;
    }
  } else if (node->kind == ND_SUB) {
    switch (node->type->kind) {
    case INT:
      printf("  sub %s, %s\n", RAX[2], RDI[2]);
      break;
    case PTR:
      if (node->rhs->type->kind != PTR) {
        printf("  imul %s, %d\n", RDI[3], node->lhs->type->ptr_to->size);
      }
      printf("  sub %s, %s\n", RAX[3], RDI[3]);
      break;
    }
  } else if (node->kind == ND_MUL) {
    switch (node->type->kind) {
    case INT:
      printf("  imul %s, %s\n", RAX[2], RDI[2]);
      break;
    case PTR:
      // unreachable
      assert(0 && "ポインタ型同士の乗算");
      break;
    }
  } else if (node->kind == ND_DIV) {
    switch (node->type->kind) {
    case INT:
      printf("  cdq\n"); // rax, rdiならcqo
      printf("  idiv %s, %s\n", RAX[2], RDI[2]);
      break;
    case PTR:
      // unreachable
      assert(0 && "ポインタ型同士の除算");
      break;
    }
  } else if (node->kind == ND_MOD) {
    switch (node->type->kind) {
    case INT:
      printf("  cdq\n"); // rax, rdiならcqo
      printf("  idiv %s, %s\n", RAX[2], RDI[2]);
      printf("  mov  %s, %s\n", RAX[2], RDX[2]);
      break;
    case PTR:
      // unreachable
      assert(0 && "ポインタ型同士のmod演算");
      break;
    }
  } else if (node->kind == ND_EQ) {
    switch (node->type->kind) {
    case INT:
      printf("  cmp %s, %s\n", RAX[2], RDI[2]);
      printf("  sete al\n");
      printf("  movzx %s, al\n", RAX[2]);
      break;
    case PTR:
      printf("  cmp %s, %s\n", RAX[3], RDI[3]);
      printf("  sete al\n");
      printf("  movzx %s, al\n", RAX[3]);
      break;
    }
  } else if (node->kind == ND_NE) {
    switch (node->type->kind) {
    case INT:
      printf("  cmp %s, %s\n", RAX[2], RDI[2]);
      printf("  setne al\n");
      printf("  movzx %s, al\n", RAX[2]);
      break;
    case PTR:
      printf("  cmp %s, %s\n", RAX[3], RDI[3]);
      printf("  setne al\n");
      printf("  movzx %s, al\n", RAX[3]);
      break;
    }
  } else if (node->kind == ND_LT) {
    switch (node->type->kind) {
    case INT:
      printf("  cmp %s, %s\n", RAX[2], RDI[2]);
      printf("  setl al\n");
      printf("  movzx %s, al\n", RAX[2]);
      break;
    case PTR:
      printf("  cmp %s, %s\n", RAX[3], RDI[3]);
      printf("  setl al\n");
      printf("  movzx %s, al\n", RAX[3]);
      break;
    }
  } else if (node->kind == ND_LE) {
    switch (node->type->kind) {
    case INT:
      printf("  cmp %s, %s\n", RAX[2], RDI[2]);
      printf("  setle al\n");
      printf("  movzx %s, al\n", RAX[2]);
      break;
    case PTR:
      printf("  cmp %s, %s\n", RAX[3], RDI[3]);
      printf("  setle al\n");
      printf("  movzx %s, al\n", RAX[3]);
      break;
    }
  }
}

void codegen(struct Node *node) {
  for (; node; node = node->rhs) {
    gen_stmt(node->lhs);
  }
}

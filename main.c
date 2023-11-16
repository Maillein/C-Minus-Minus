#include "cmm.h"

// ユーザの入力
char *user_input;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません．\n");
    return 1;
  }

  user_input = argv[1];
  // トークナイズする
  struct Token *token = tokenize();
  // 抽象構文木の生成
  program(&token);

  // アセンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");  // 変数26個分の領域

  // 先頭の式から順にコード生成
  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果をRAXに
    printf("  pop rax\n");
  }

  // エピローグ
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

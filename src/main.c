#include "cmm.h"
#include <stdio.h>

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
  struct ASTNode *program = parse(&token);

  // ASTの出力
  vis_ast(program);

  // アセンブリの前半を出力
  printf(".intel_syntax noprefix\n");

  codegen(program);

  return 0;
}

#include "9cc.h"

// ユーザの入力
char *user_input;
// 現在着目しているトークン
struct Token *token;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません．\n");
    return 1;
  }

  user_input = argv[1];
  // トークナイズする
  token = tokenize();
  // 抽象構文木の生成
  struct Node *node = expr();

  // アセンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 抽象構文木を下りながらコードを生成
  gen(node);

  // スタックトップに式全体の値が残っているので，
  // それをRAXにロード
  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}

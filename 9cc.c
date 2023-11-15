#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

// トークン型
struct Token {
  TokenKind kind;     // トークンの型
  struct Token *next; // 次の入力トークン
  int val;            // kindがTK_NUMの場合，その整数
  char *str;          // トークン文字列
};

// 現在着目しているトークン
struct Token *token;

// エラーを報告するための関数
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときは，トークンを1個読み進めて
// 真を返す．それ以外のときは偽を返す．
bool consume(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときは，トークンを1個読み進める．
// それ以外の場合はエラーを報告する．
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error("'%c'ではありません", op);
  }
  token = token->next;
}

// 次のトークンが数値の場合，トークンを1個読み進めてその数値を返す．
// それ以外の場合はエラーを報告する．
int expect_number() {
  if (token->kind != TK_NUM) {
    error("数値ではありません");
  }
  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

// 新しいトークンを作成して，curにつなげる．
struct Token *new_token(TokenKind kind, struct Token *cur, char *str) {
  struct Token *tok = calloc(1, sizeof(struct Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

struct Token *tokenize(char *p) {
  struct Token head;
  head.next = NULL;
  struct Token *cur = &head;

  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error("トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません．\n");
    return 1;
  }

  // トークナイズする
  token = tokenize(argv[1]);

  // アセンブリの前半を出力
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // 式の最初は数値でなければならないので，それをチェックして
  // 最初のmov命令を出力
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {
    if (consume('+')) {
      printf("  add rax, %d\n", expect_number());
    }

    expect('-');
    printf("  sub rax, %d\n", expect_number());
  }

  printf("  ret\n");
  return 0;
}

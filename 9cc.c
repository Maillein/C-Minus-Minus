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

// ユーザの入力
char *user_input;

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

// エラー発生箇所を報告するための関数
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, "");
  fprintf(stderr, "^ ");
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
    error_at(token->str, "'%c'ではありません", op);
  }
  token = token->next;
}

// 次のトークンが数値の場合，トークンを1個読み進めてその数値を返す．
// それ以外の場合はエラーを報告する．
int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数値ではありません");
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

struct Token *tokenize() {
  char *p = user_input;
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

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

// 抽象構文木のノードの種類
typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_NUM, // 整数
} NodeKind;

// 抽象構文木のノード型
struct Node {
  NodeKind kind;
  struct Node *lhs;
  struct Node *rhs;
  int val;
};

struct Node *new_node(NodeKind kind, struct Node *lhs, struct Node *rhs) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

struct Node *new_node_num(int val) {
  struct Node *node = calloc(1, sizeof(struct Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

struct Node *expr() {
  struct Node *node = new_node_num(expect_number());

  for (;;) {
    if (consume('+')) {
      node = new_node(ND_ADD, node, expr());
    } else if (consume('-')) {
      node = new_node(ND_SUB, node, expr());
    } else {
      return node;
    }
  }
}

void gen(struct Node *node) {
  if (node->kind == ND_NUM) {
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    default:
      break;
  }

  printf("  push rax\n");
}

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

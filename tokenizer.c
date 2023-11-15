#include "9cc.h"

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
bool consume(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(op, token->str, token->len)) {
    return false;
  }
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときは，トークンを1個読み進める．
// それ以外の場合はエラーを報告する．
void expect(char *op) {
  if (token->kind != TK_RESERVED || strlen(op) != token->len ||
      memcmp(op, token->str, token->len)) {
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

bool at_eof() { return token->kind == TK_EOF; }

// 新しいトークンを作成して，curにつなげる．
struct Token *new_token(TokenKind kind, struct Token *cur, char *str, int len) {
  struct Token *tok = calloc(1, sizeof(struct Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
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

    if (memcmp(p, "<=", 2) == 0 || memcmp(p, ">=", 2) == 0 ||
        memcmp(p, "==", 2) == 0 || memcmp(p, "!=", 2) == 0) {
      cur = new_token(TK_RESERVED, cur, p, 2);
      p += 2;
      continue;
    }

    if (strchr("+-*/()<>", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      char *old_p = p;
      cur->val = strtol(p, &p, 10);
      cur->len = (int)(p - old_p);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p, 0);
  return head.next;
}

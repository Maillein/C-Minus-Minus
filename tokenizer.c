#include "cmm.h"

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

// トークンの文字列が期待しているものと等しいか？
bool equal(struct Token **tok, char *op) {
  if (strlen(op) == (*tok)->len && memcmp((*tok)->str, op, (*tok)->len) == 0) {
    *tok = (*tok)->next;
    return true;
  }
  return false;
}

// 次のトークンが期待している記号のときは，トークンを1個読み進める．
// それ以外の場合はエラーを報告する．
struct Token *skip(struct Token **tok, char *op) {
  if (!equal(tok, op)) {
    error_at((*tok)->str, "'%s'ではありません", op);
  }
  return *tok;
}

// 次のトークンの型がkindのときは，トークンを1個読み進めて
// 真を返す．それ以外のときは偽を返す．
bool consume(struct Token **tok, enum TokenKind kind) {
  if ((*tok)->kind != kind) {
    return false;
  }
  *tok = (*tok)->next;
  return true;
}

bool at_eof(struct Token **tok) { return (*tok)->kind == TK_EOF; }

// 新しいトークンを作成して，curにつなげる．
struct Token *new_token(enum TokenKind kind, struct Token *cur, char *str, int len) {
  struct Token *tok = calloc(1, sizeof(struct Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

int is_alnum(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || c == '_';
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

    if (strchr(";+-*/()<>=", *p)) {
      cur = new_token(TK_RESERVED, cur, p++, 1);
      continue;
    }

    if (memcmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if ('a' <= *p && *p <= 'z' || 'A' <= *p && *p <= 'Z' || *p == '_') {
      cur = new_token(TK_IDENT, cur, p, 0);
      while (is_alnum(*p)) {
        p++;
        cur->len++;
      }
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

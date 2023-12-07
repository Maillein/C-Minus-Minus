#include "cmm.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *TokenKind2str(enum TokenKind kind) {
  switch (kind) {
  case TK_EXCLA:
    return "!";
  case TK_DQUOTE:
    return "\"";
  case TK_HASH:
    return "#";
  case TK_DOLLER:
    return "$";
  case TK_PERCENT:
    return "%";
  case TK_AMP:
    return "&";
  case TK_DAMP:
    return "&&";
  case TK_SQUOTE:
    return "'";
  case TK_L_PAREN:
    return "(";
  case TK_R_PAREN:
    return ")";
  case TK_ASTER:
    return "*";
  case TK_PLUS:
    return "+";
  case TK_DPLUS:
    return "++";
  case TK_COMMA:
    return ",";
  case TK_MINUS:
    return "-";
  case TK_DMINUS:
    return "--";
  case TK_ALLOW:
    return "->";
  case TK_PERIOD:
    return ".";
  case TK_SLASH:
    return "/";
  case TK_DSLASH:
    return "//";
  case TK_SLA_AST:
    return "/*";
  case TK_AST_SLA:
    return "*/";
  case TK_COLON:
    return ":";
  case TK_SEMICOLON:
    return ";";
  case TK_EQUAL:
    return "=";
  case TK_DEQUAL:
    return "==";
  case TK_NEQUAL:
    return "!=";
  case TK_PLUSEQUAL:
    return "+=";
  case TK_MINUSEQUAL:
    return "-=";
  case TK_LT:
    return "<";
  case TK_LE:
    return "<=";
  case TK_GT:
    return ">";
  case TK_GE:
    return "<=";
  case TK_L_SBRACKET:
    return "[";
  case TK_R_SBRACKET:
    return "]";
  case TK_BACKSLASH:
    return "\\";
  case TK_CARET:
    return "^";
  case TK_L_CBRACKET:
    return "{";
  case TK_R_CBRACKET:
    return "}";
  case TK_VBAR:
    return "|";
  case TK_DVBAR:
    return "||";
  case TK_TILDE:
    return "~";
  case TK_USCORE:
    return "_";
  case TK_SIZEOF:
    return "sizeof";
  case TK_IF:
    return "if";
  case TK_ELSE:
    return "else";
  case TK_FOR:
    return "for";
  case TK_WHILE:
    return "while";
  case TK_RETURN:
    return "return";
  case TK_INT:
    return "int";
  case TK_IDENT:
    return "Identifier";
  case TK_INTEGER:
    return "Integer";
  case TK_EOF:
    return "EOF";
    break;
  }
  return "";
}

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

// 次のトークンが期待しているkindのときは，トークンを1個読み進める．
// それ以外の場合はエラーを報告する．
bool expect(struct Token **tok, enum TokenKind kind) {
  if ((*tok)->kind != kind) {
    error_at((*tok)->str, "'%s'ではありません", TokenKind2str(kind));
    return false;
  }
  *tok = (*tok)->next;
  return true;
}

// 次のトークンが期待しているkindのときは，トークンを1個読み進め真を返す．
// それ以外のときは偽を返す．
bool consume(struct Token **tok, enum TokenKind kind) {
  if ((*tok)->kind != kind) {
    return false;
  }
  *tok = (*tok)->next;
  return true;
}

bool at_eof(struct Token **tok) { return (*tok)->kind == TK_EOF; }

// 新しいトークンを作成して，curにつなげる．
struct Token *new_token(enum TokenKind kind, struct Token *cur, char *str,
                        int len) {
  struct Token *tok = calloc(1, sizeof(struct Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

int is_ident_char1(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_';
}

int is_ident_char2(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
         ('0' <= c && c <= '9') || c == '_';
}

int is_keyword(char *p, char *s, int s_len) {
  return memcmp(p, s, s_len) == 0 && !is_ident_char2(p[s_len]);
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
    if (memcmp(p, "<=", 2) == 0) {
      cur = new_token(TK_LE, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, ">=", 2) == 0) {
      cur = new_token(TK_GE, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "==", 2) == 0) {
      cur = new_token(TK_DEQUAL, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "!=", 2) == 0) {
      cur = new_token(TK_NEQUAL, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "+=", 2) == 0) {
      cur = new_token(TK_PLUSEQUAL, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "-=", 2) == 0) {
      cur = new_token(TK_MINUSEQUAL, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "&&", 2) == 0) {
      cur = new_token(TK_DAMP, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "||", 2) == 0) {
      cur = new_token(TK_DVBAR, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "++", 2) == 0) {
      cur = new_token(TK_DPLUS, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "--", 2) == 0) {
      cur = new_token(TK_DMINUS, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "->", 2) == 0) {
      cur = new_token(TK_ALLOW, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "//", 2) == 0) {
      cur = new_token(TK_DSLASH, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "/*", 2) == 0) {
      cur = new_token(TK_SLA_AST, cur, p, 2);
      p += 2;
      continue;
    }
    if (memcmp(p, "*/", 2) == 0) {
      cur = new_token(TK_AST_SLA, cur, p, 2);
      p += 2;
      continue;
    }

    if (memcmp(p, "!", 1) == 0) {
      cur = new_token(TK_EXCLA, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "\"", 1) == 0) {
      cur = new_token(TK_DQUOTE, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "#", 1) == 0) {
      cur = new_token(TK_HASH, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "$", 1) == 0) {
      cur = new_token(TK_DOLLER, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "%", 1) == 0) {
      cur = new_token(TK_PERCENT, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "&", 1) == 0) {
      cur = new_token(TK_AMP, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "'", 1) == 0) {
      cur = new_token(TK_SQUOTE, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "(", 1) == 0) {
      cur = new_token(TK_L_PAREN, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ")", 1) == 0) {
      cur = new_token(TK_R_PAREN, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "*", 1) == 0) {
      cur = new_token(TK_ASTER, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "+", 1) == 0) {
      cur = new_token(TK_PLUS, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ",", 1) == 0) {
      cur = new_token(TK_COMMA, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "-", 1) == 0) {
      cur = new_token(TK_MINUS, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ".", 1) == 0) {
      cur = new_token(TK_PERIOD, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "/", 1) == 0) {
      cur = new_token(TK_SLASH, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ":", 1) == 0) {
      cur = new_token(TK_COLON, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ";", 1) == 0) {
      cur = new_token(TK_SEMICOLON, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "=", 1) == 0) {
      cur = new_token(TK_EQUAL, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "<", 1) == 0) {
      cur = new_token(TK_LT, cur, p++, 1);
      continue;
    }
    if (memcmp(p, ">", 1) == 0) {
      cur = new_token(TK_GT, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "[", 1) == 0) {
      cur = new_token(TK_L_SBRACKET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "]", 1) == 0) {
      cur = new_token(TK_R_SBRACKET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "\\", 1) == 0) {
      cur = new_token(TK_BACKSLASH, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "^", 1) == 0) {
      cur = new_token(TK_CARET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "{", 1) == 0) {
      cur = new_token(TK_L_CBRACKET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "}", 1) == 0) {
      cur = new_token(TK_R_CBRACKET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "|", 1) == 0) {
      cur = new_token(TK_R_CBRACKET, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "~", 1) == 0) {
      cur = new_token(TK_TILDE, cur, p++, 1);
      continue;
    }
    if (memcmp(p, "_", 1) == 0) {
      cur = new_token(TK_USCORE, cur, p++, 1);
      continue;
    }


    if (is_keyword(p, "sizeof", 6)) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
      p += 6;
      continue;
    }
    if (is_keyword(p, "return", 6)) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }
    if (is_keyword(p, "if", 2)) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }
    if (is_keyword(p, "else", 4)) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }
    if (is_keyword(p, "while", 5)) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }
    if (is_keyword(p, "for", 3)) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }
    if (is_keyword(p, "int", 3)) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if (is_ident_char1(*p)) {
      cur = new_token(TK_IDENT, cur, p, 0);
      while (is_ident_char2(*p)) {
        p++;
        cur->len++;
      }
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_INTEGER, cur, p, 1);
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

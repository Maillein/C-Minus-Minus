#!/bin/zsh
assert() {
  expected="$1"
  input="$2"

  build/cmm "$input" > test/tmp.s
  gcc -c -o test/test_functions.o test/test_functions.c
  gcc -g -o test/tmp test/tmp.s test/test_functions.o
  ./test/tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'int main() { return 0; }'
assert 42 'int main() { return 42; }'
assert 21 'int main() { return 5+20-4; }'
assert 41 'int main() { return  12 + 34 - 5 ; }'
assert 5 'int main() { return 1 * 2 + 3; }'
assert 14 'int main() { return 1 * 2 + 3 * 4; }'
assert 16 'int main() { return 10 + 3 * 2; }'
assert 10 'int main() { return 7 + 6 / 2; }'
assert 20 'int main() { return 4 * (3 + 2); }'
assert 4 'int main() { return (8 + 4) / 3; }'
assert 10 'int main() { return -10 + 20; }'
assert 16 'int main() { return -4*-4; }'
assert 3 'int main() { return 10 % 7; }'

assert 0 'int main() { return 0==1; }'
assert 1 'int main() { return 42==42; }'
assert 1 'int main() { return 0!=1; }'
assert 0 'int main() { return 42!=42; }'

assert 1 'int main() { return 0<1; }'
assert 0 'int main() { return 1<1; }'
assert 0 'int main() { return 2<1; }'
assert 1 'int main() { return 0<=1; }'
assert 1 'int main() { return 1<=1; }'
assert 0 'int main() { return 2<=1; }'

assert 1 'int main() { return 1>0; }'
assert 0 'int main() { return 1>1; }'
assert 0 'int main() { return 1>2; }'
assert 1 'int main() { return 1>=0; }'
assert 1 'int main() { return 1>=1; }'
assert 0 'int main() { return 1>=2; }'

assert 1 'int main() { int a; return a=1; }'
assert 7 'int main() { int a; int b; int c; int d; a=7;b=a;c=b; return d=c; }'
assert 64 'int main() { int a; int z; a=60;z=4; return a+z; }'
assert 9 'int main() { int foo; return foo=9; }'
assert 15 'int main() { int foo; int bar; foo=20;bar=5; return foo-bar; }'
assert 12 'int main() { int a_1; int a_2; int a_3; int a_4; a_1 = 2; a_2 = 2; a_3 = 3; a_4 = a_1 * a_2 * a_3; return a_4; }'
assert 1 'int main() { int a; int b; int c; int d; int e; int f; int g; a = b = c = d = e = f = g = 1; return a; }'
assert 4 'int main() { int a; int b; int c; int d; a = (b = (c = (d = 1) + 1) + 1) + 1; if (d == 1) return a; else return 100;}'
assert 1 'int main() { int a; int b; return a = (b = 1); }'

assert 1 'int main() { return 1; 2; 3; }'
assert 2 'int main() { 1; return 2; 3; }'
assert 3 'int main() { 1; 2; return 3; }'

assert 1 'int main() { if (1) return 1; return 2; }'
assert 2 'int main() { if (0) return 1; return 2; }'
assert 1 'int main() { int a; a = 1; if (a) return 1; return 2; }'
assert 2 'int main() { int a; a = 0; if (a) return 1; return 2; }'
assert 1 'int main() { int a; int b; a = 0; b = 1; if (a + b) return 1; return 2; }'
assert 2 'int main() { int a; int b; a = 0; b = 0; if (a + b) return 1; return 2; }'
assert 1 'int main() { if (1) if (1) return 1; return 2; }'
assert 2 'int main() { if (1) if (0) return 1; return 2; }'
assert 2 'int main() { if (0) if (1) return 1; return 2; }'
assert 2 'int main() { if (0) if (0) return 1; return 2; }'

assert 1 'int main() { if (1) return 1; else return 2; return 3; }'
assert 2 'int main() { if (0) return 1; else return 2; return 3; }'
assert 1 'int main() { if (1) { if (1) { return 1; } else { return 2; } } else { if (1) { return 3; } else { return 4; } } return 5; }'
assert 1 'int main() { if (1) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 'int main() { if (1) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 2 'int main() { if (1) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 'int main() { if (0) if (1) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 'int main() { if (0) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 'int main() { if (0) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 'int main() { if (0) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 '
int main() {
  int a;
  a = 2;
  if (a == 1) {
    return 1;
  } else if (a == 2) { 
    return 2;
  } else if (a == 3) {
    return 3;
  } else {
    return 4;
  }
}
'

assert 5 'int main() { int a; a=10; while (a > 5) a = a - 1; return a; }'
assert 55 'int main() { int i; int j; i = 0; j = 0; for (i = 0; i <= 10; i = i + 1) j = i + j; return j; }'
assert 55 'int main() { int i; int j; i = 0; j = 0; for (; i <= 10; i = i + 1) j = i + j; return j; }'
assert 11 'int main() { int i; i = 0; for (; i <= 10;) i = i + 1; return i; }'
assert 34 'int main() { for (;;) return 34; }'

assert 5 'int main() { { { return 5; } } }'
assert 2 'int main() { 1; { return 2; } return 3; }'
assert 3 'int main() { 1; { 2; } return 3; }'
# assert 2 'int main() { 1; } { return 2; } { 3; }'
# assert 3 'int main() { 1; } { 2; } { 3; }'
assert 45 '
int main() {
  int i;
  int sum;
  i = 100;
  sum = 0;
  for (i = 0; i <= 10; i = i + 1) { 
    put_num(i);
    if (i >= 5) { 
      sum = sum + i;
    }
  } 
  return sum;
}'

assert 10 'int main() { ;; int a; a = 10;; ; ; return arg1(a); }'

assert 0 'int main() { hello(); }'
assert 19 'int main() { return arg0(); }'
assert 20 'int main() { return arg0() + 1; }'
assert 10 'int main() { int a; a = arg0(); return 10; }'
assert 19 'int main() { return hello() + arg0(); }'

assert 1 'int main() { return arg1(1); }'
assert 3 'int main() { return arg2(1, 2); }'
assert 6 'int main() { return arg3(1, 2, 3); }'
assert 10 'int main() { return arg4(1, 2, 3, 4); }'
assert 15 'int main() { return arg5(1, 2, 3, 4, 5); }'
assert 21 'int main() { return arg6(1, 2, 3, 4, 5, 6); }'
assert 0 'int main() { return print6(1, 2, 3, 4, 5, 6); }'
# assert 6 'int main() { a = b = c = d = e = f = 1; return arg6(a, b, c, d, e, f); }'
# assert 10 'int main() { a = b = c = d = e = f = 1; return arg6(a, b + c, c + d + e, d, h = (g = e = 1), arg2(1, 1)); }'
assert 1 'int main() { return arg1(arg1(arg1(arg1(1)))); }'

assert 55 'int main() { 
  int s; int i;
  i = 0;
  s = 0; 
  { int i; i = 10; my_assert(10, i); }
  { int i; i = 20; my_assert(20, i); } 
  my_assert(0, s);
  for (i = 0; i <= 10; i = i + 1) {
    s = s + i;
  }
  i = 30;
  my_assert(30, i);
  return s;
}'

# 変数宣言の構文が追加されたら，以下2個は通るようになるはず
assert 20 'int main() { int i; i = 10; { int i; i = 20; return i; } return 30; }'
assert 4 'int main() { int a; a = 4; { int a; a = 15; put_num(a); } { put_num(a); } return a; }'

assert 13 'int foo(int x, int y) { int z; z = 10; return x + y + z; } int main() { int x; int y; x = 1; y = 2; return foo(x, y); }'
assert 0 '
int fib(int x) {
  if (x == 0) {
    return 0;
  } else if (x == 1) {
    return 1;
  } else {
    return fib(x - 1) + fib(x - 2);
  }
}
int main() {
  int i;
  for (i = 0; i < 20; i = i + 1) {
    put_num(fib(i));
  }
  return 0;
}
'

assert 12 '
int tarai(int x, int y, int z) {
  if (x <= y) {
    return y;
  } else {
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
  }
}
int main() {
  int x; int y; int z;
  x = 12;
  y = 6;
  z = 0;
  return tarai(x, y, z);
}
'

assert 3 '
int mean6(int a, int b, int c, int d, int e, int f) {
  int s1; int s2; int s3; int s4;
  s1 = a + b;
  s2 = c + d;
  print6(a, b, c, d, e, f);
  s3 = e + f;
  s4 = s1 + s2 + s3;
  return s4 / 6;
}
int main() {
  int a; int b;
  a = 2;
  b = 3;
  return mean6(b - a, a, b, a + a, a + b, a * b);
}
'

assert 2 'int main() { if (put_num(0) && put_num(0)) return 1; else return 2; }'
assert 2 'int main() { if (put_num(0) && put_num(456)) return 1; else return 2; }'
assert 2 'int main() { if (put_num(123) && put_num(0)) return 1; else return 2; }'
assert 1 'int main() { if (put_num(123)&&put_num(456)) return 1; else return 2; }'

assert 2 'int main() { if (put_num(0) || put_num(0)) return 1; else return 2; }'
assert 1 'int main() { if (put_num(0) || put_num(456)) return 1; else return 2; }'
assert 1 'int main() { if (put_num(123) || put_num(0)) return 1; else return 2; }'
assert 1 'int main() { if (put_num(123)||put_num(456)) return 1; else return 2; }'

assert 1 'int main() { return put_num(1) && put_num(2) && put_num(3); }'
assert 0 'int main() { return put_num(1) && put_num(0) && put_num(3); }'

assert 0 '
int fizbuz(int x) { 
  int i;
  for (i = 1; i <= x; i = i + 1) {
    if (i % 3 == 0 && i % 5 == 0) p_fizbuz();
    else if (i % 3 == 0)  p_fiz();
    else if (i % 5 == 0)  p_buz();
    else                  put_num(i);
  }
  return 0; 
}
int main() { return fizbuz(30); }'

assert 3 'int main() { int x; int y; int *z; x = 3; y = 5; z = &y + 8; put_num(&x); put_num(z); return *z; }'
assert 3 'int main() { int x; int y; x = 3; y = 5; return *(&y + 8); }'
assert 5 'int main() { int x; int y; int z; x = 3; y = 5; z = 7; return *(&y); }'
assert 2 'int main() { int x; int *y; y = &x; put_num(&x); put_num(y); *y = 2; return x;}'
assert 2 'int main() { int x; int *y; int **z; y = &x; z = &y; **z = 2; return x;}'

assert 49 'int inc(int *x) { *x = *x + 1; } int main() { int x; x = 48; inc(&x); return x; }'
assert 17 'int add2(int x, int y, int* z) { *z = x + y; } int main() { int a; add2(14, 3, &a); return a; }'

echo OK

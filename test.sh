#!/bin/zsh
assert() {
  expected="$1"
  input="$2"

  ./cmm "$input" > tmp.s
  gcc -c -o test_functions.o test_functions.c
  gcc -g -o tmp tmp.s test_functions.o
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 'main() { return 0; }'
assert 42 'main() { return 42; }'
assert 21 'main() { return 5+20-4; }'
assert 41 'main() { return  12 + 34 - 5 ; }'
assert 5 'main() { return 1 * 2 + 3; }'
assert 14 'main() { return 1 * 2 + 3 * 4; }'
assert 16 'main() { return 10 + 3 * 2; }'
assert 10 'main() { return 7 + 6 / 2; }'
assert 20 'main() { return 4 * (3 + 2); }'
assert 4 'main() { return (8 + 4) / 3; }'
assert 10 'main() { return -10 + 20; }'
assert 16 'main() { return -4*-4; }'

assert 0 'main() { return 0==1; }'
assert 1 'main() { return 42==42; }'
assert 1 'main() { return 0!=1; }'
assert 0 'main() { return 42!=42; }'

assert 1 'main() { return 0<1; }'
assert 0 'main() { return 1<1; }'
assert 0 'main() { return 2<1; }'
assert 1 'main() { return 0<=1; }'
assert 1 'main() { return 1<=1; }'
assert 0 'main() { return 2<=1; }'

assert 1 'main() { return 1>0; }'
assert 0 'main() { return 1>1; }'
assert 0 'main() { return 1>2; }'
assert 1 'main() { return 1>=0; }'
assert 1 'main() { return 1>=1; }'
assert 0 'main() { return 1>=2; }'

assert 1 'main() { return a=1; }'
assert 7 'main() { a=7;b=a;c=b; return d=c; }'
assert 64 'main() { a=60;z=4; return a+z; }'
assert 9 'main() { return foo=9; }'
assert 15 'main() { foo=20;bar=5; return foo-bar; }'
assert 12 'main() { a_1 = 2; a_2 = 2; a_3 = 3; a_4 = a_1 * a_2 * a_3; return a_4; }'
assert 1 'main() { a = b = c = d = e = f = g = 1; return a; }'
assert 4 'main() { a = (b = (c = (d = 1) + 1) + 1) + 1; if (d == 1) return a; else return 100;}'
assert 1 'main() { return a = (b = 1); }'

assert 1 'main() { return 1; 2; 3; }'
assert 2 'main() { 1; return 2; 3; }'
assert 3 'main() { 1; 2; return 3; }'

assert 1 'main() { if (1) return 1; return 2; }'
assert 2 'main() { if (0) return 1; return 2; }'
assert 1 'main() { a = 1; if (a) return 1; return 2; }'
assert 2 'main() { a = 0; if (a) return 1; return 2; }'
assert 1 'main() { a = 0; b = 1; if (a + b) return 1; return 2; }'
assert 2 'main() { a = 0; b = 0; if (a + b) return 1; return 2; }'
assert 1 'main() { if (1) if (1) return 1; return 2; }'
assert 2 'main() { if (1) if (0) return 1; return 2; }'
assert 2 'main() { if (0) if (1) return 1; return 2; }'
assert 2 'main() { if (0) if (0) return 1; return 2; }'

assert 1 'main() { if (1) return 1; else return 2; return 3; }'
assert 2 'main() { if (0) return 1; else return 2; return 3; }'
assert 1 'main() { if (1) { if (1) { return 1; } else { return 2; } } else { if (1) { return 3; } else { return 4; } } return 5; }'
assert 1 'main() { if (1) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 'main() { if (1) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 2 'main() { if (1) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 'main() { if (0) if (1) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 'main() { if (0) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 'main() { if (0) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 'main() { if (0) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 '
main() {
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

assert 5 'main() { a=10; while (a > 5) a = a - 1; return a; }'
assert 55 'main() { i = 0; j = 0; for (i = 0; i <= 10; i = i + 1) j = i + j; return j; }'
assert 55 'main() { i = 0; j = 0; for (; i <= 10; i = i + 1) j = i + j; return j; }'
assert 11 'main() { i = 0; for (; i <= 10;) i = i + 1; return i; }'
assert 34 'main() { for (;;) return 34; }'

assert 5 'main() { { { return 5; } } }'
assert 2 'main() { 1; { return 2; } return 3; }'
assert 3 'main() { 1; { 2; } return 3; }'
# assert 2 'main() { 1; } { return 2; } { 3; }'
# assert 3 'main() { 1; } { 2; } { 3; }'
assert 45 '
main() {
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

assert 10 'main() { ;; a = 10;; ; ; return arg1(a); }'

assert 0 'main() { hello(); }'
assert 19 'main() { return arg0(); }'
assert 20 'main() { return arg0() + 1; }'
assert 10 'main() { a = arg0(); return 10; }'
assert 19 'main() { return hello() + arg0(); }'

assert 1 'main() { return arg1(1); }'
assert 3 'main() { return arg2(1, 2); }'
assert 6 'main() { return arg3(1, 2, 3); }'
assert 10 'main() { return arg4(1, 2, 3, 4); }'
assert 15 'main() { return arg5(1, 2, 3, 4, 5); }'
assert 21 'main() { return arg6(1, 2, 3, 4, 5, 6); }'
assert 0 'main() { return print6(1, 2, 3, 4, 5, 6); }'
assert 6 'main() { a = b = c = d = e = f = 1; return arg6(a, b, c, d, e, f); }'
assert 10 'main() { a = b = c = d = e = f = 1; return arg6(a, b + c, c + d + e, d, h = (g = e = 1), arg2(1, 1)); }'
assert 1 'main() { return arg1(arg1(arg1(arg1(1)))); }'

assert 55 'main() { 
  s = 0; 
  { i = 10; my_assert(10, i); }
  { i = 20; my_assert(20, i); } 
  my_assert(0, s);
  for (i = 0; i <= 10; i = i + 1) {
    s = s + i;
  }
  i = 30;
  my_assert(30, i);
  return s;
}'

# 変数宣言の構文が追加されたら，以下2個は通るようになるはず
# assert 20 'main() { i = 10; { i = 20; return i; } return 30; }'
# assert 4 'main() { a = 4; { a = 15; put_num(a); } { put_num(a); } return a; }'

assert 13 'foo(x,y) { z = 10; return x + y + z; } main() { x = 1; y = 2; return foo(x, y); }'
assert 0 '
fib(x) {
  if (x == 0) {
    return 0;
  } else if (x == 1) {
    return 1;
  } else {
    return fib(x - 1) + fib(x - 2);
  }
}
main() {
  for (i = 0; i < 20; i = i + 1) {
    put_num(fib(i));
  }
  return 0;
}
'

assert 12 '
tarai(x, y, z) {
  if (x <= y) {
    return y;
  } else {
    return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y));
  }
}
main() {
  x = 12;
  y = 6;
  z = 0;
  return tarai(x, y, z);
}
'

assert 3 '
mean6(a, b, c, d, e, f) {
  s1 = a + b;
  s2 = c + d;
  print6(a, b, c, d, e, f);
  s3 = e + f;
  s4 = s1 + s2 + s3;
  return s4 / 6;
}
main() {
  a = 2;
  b = 3;
  return mean6(b - a, a, b, a + a, a + b, a * b);
}
'

echo OK

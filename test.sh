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

assert 0 '{ return 0; }'
assert 42 '{ return 42; }'
assert 21 '{ return 5+20-4; }'
assert 41 '{ return  12 + 34 - 5 ; }'
assert 5 '{ return 1 * 2 + 3; }'
assert 14 '{ return 1 * 2 + 3 * 4; }'
assert 16 '{ return 10 + 3 * 2; }'
assert 10 '{ return 7 + 6 / 2; }'
assert 20 '{ return 4 * (3 + 2); }'
assert 4 '{ return (8 + 4) / 3; }'
assert 10 '{ return -10 + 20; }'
assert 16 '{ return -4*-4; }'

assert 0 '{ return 0==1; }'
assert 1 '{ return 42==42; }'
assert 1 '{ return 0!=1; }'
assert 0 '{ return 42!=42; }'

assert 1 '{ return 0<1; }'
assert 0 '{ return 1<1; }'
assert 0 '{ return 2<1; }'
assert 1 '{ return 0<=1; }'
assert 1 '{ return 1<=1; }'
assert 0 '{ return 2<=1; }'

assert 1 '{ return 1>0; }'
assert 0 '{ return 1>1; }'
assert 0 '{ return 1>2; }'
assert 1 '{ return 1>=0; }'
assert 1 '{ return 1>=1; }'
assert 0 '{ return 1>=2; }'

assert 1 '{ return a=1; }'
assert 7 '{ a=7;b=a;c=b; return d=c; }'
assert 64 '{ a=60;z=4; return a+z; }'
assert 9 '{ return foo=9; }'
assert 15 '{ foo=20;bar=5; return foo-bar; }'
assert 12 '{ a_1 = 2; a_2 = 2; a_3 = 3; a_4 = a_1 * a_2 * a_3; return a_4; }'
assert 1 '{ a = b = c = d = e = f = g = 1; return a; }'
assert 4 '{ a = (b = (c = (d = 1) + 1) + 1) + 1; if (d == 1) return a; else return 100;}'
assert 1 '{ return a = (b = 1); }'

assert 1 '{ return 1; 2; 3; }'
assert 2 '{ 1; return 2; 3; }'
assert 3 '{ 1; 2; return 3; }'

assert 1 '{ if (1) return 1; return 2; }'
assert 2 '{ if (0) return 1; return 2; }'
assert 1 '{ a = 1; if (a) return 1; return 2; }'
assert 2 '{ a = 0; if (a) return 1; return 2; }'
assert 1 '{ a = 0; b = 1; if (a + b) return 1; return 2; }'
assert 2 '{ a = 0; b = 0; if (a + b) return 1; return 2; }'
assert 1 '{ if (1) if (1) return 1; return 2; }'
assert 2 '{ if (1) if (0) return 1; return 2; }'
assert 2 '{ if (0) if (1) return 1; return 2; }'
assert 2 '{ if (0) if (0) return 1; return 2; }'

assert 1 '{ if (1) return 1; else return 2; return 3; }'
assert 2 '{ if (0) return 1; else return 2; return 3; }'
assert 1 '{ if (1) { if (1) { return 1; } else { return 2; } } else { if (1) { return 3; } else { return 4; } } return 5; }'
assert 1 '{ if (1) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 '{ if (1) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 2 '{ if (1) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 '{ if (0) if (1) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 '{ if (0) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 3 '{ if (0) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5; }'
assert 4 '{ if (0) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5; }'
assert 2 '
{
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

assert 5 '{ a=10; while (a > 5) a = a - 1; return a; }'
assert 55 '{ i = 0; j = 0; for (i = 0; i <= 10; i = i + 1) j = i + j; return j; }'
assert 55 '{ i = 0; j = 0; for (; i <= 10; i = i + 1) j = i + j; return j; }'
assert 11 '{ i = 0; for (; i <= 10;) i = i + 1; return i; }'
assert 34 '{ for (;;) return 34; }'

assert 5 '{ { { return 5; } } }'
assert 2 '{ 1; { return 2; } return 3; }'
assert 3 '{ 1; { 2; } return 3; }'
# assert 2 '{ 1; } { return 2; } { 3; }'
# assert 3 '{ 1; } { 2; } { 3; }'
assert 45 '
{
  sum = 0;
  for (i = 0; i <= 10; i = i + 1) { 
    if (i >= 5) { 
      sum = sum + i;
    }
  } 
  return sum;
}'

assert 0 '{ hello(); }'
assert 19 '{ return arg0(); }'
assert 20 '{ return arg0() + 1; }'
assert 10 '{ a = arg0(); return 10; }'
assert 19 '{ return hello() + arg0(); }'

assert 1 '{ return arg1(1); }'
assert 3 '{ return arg2(1, 2); }'
assert 6 '{ return arg3(1, 2, 3); }'
assert 10 '{ return arg4(1, 2, 3, 4); }'
assert 15 '{ return arg5(1, 2, 3, 4, 5); }'
assert 21 '{ return arg6(1, 2, 3, 4, 5, 6); }'
assert 0 '{ return print6(1, 2, 3, 4, 5, 6); }'
assert 6 '{ a = b = c = d = e = f = 1; return arg6(a, b, c, d, e, f); }'
assert 10 '{ a = b = c = d = e = f = 1; return arg6(a, b + c, c + d + e, d, h = (g = e = 1), arg2(1, 1)); }'
assert 1 '{ return arg1(arg1(arg1(arg1(1)))); }'

echo OK

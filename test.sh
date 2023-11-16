#!/bin/zsh
assert() {
  expected="$1"
  input="$2"

  ./cmm "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 '0;'
assert 42 '42;'
assert 21 '5+20-4;'
assert 41 ' 12 + 34 - 5 ;'
assert 5 '1 * 2 + 3;'
assert 14 '1 * 2 + 3 * 4;'
assert 16 '10 + 3 * 2;'
assert 10 '7 + 6 / 2;'
assert 20 '4 * (3 + 2);'
assert 4 '(8 + 4) / 3;'
assert 10 '-10 + 20;'
assert 16 '-4*-4;'

assert 0 '0==1;'
assert 1 '42==42;'
assert 1 '0!=1;'
assert 0 '42!=42;'

assert 1 '0<1;'
assert 0 '1<1;'
assert 0 '2<1;'
assert 1 '0<=1;'
assert 1 '1<=1;'
assert 0 '2<=1;'

assert 1 '1>0;'
assert 0 '1>1;'
assert 0 '1>2;'
assert 1 '1>=0;'
assert 1 '1>=1;'
assert 0 '1>=2;'

assert 3 '1; 2; 3;'
assert 1 'a=1;'
assert 7 'a=7;b=a;c=b;d=c;'
assert 64 'a=60;z=4;a+z;'
assert 9 'foo=9;'
assert 15 'foo=20;bar=5;foo-bar;'
assert 12 'a_1 = 2; a_2 = 2; a_3 = 3; a_4 = a_1 * a_2 * a_3;'

assert 1 'return 1; 2; 3;'
assert 2 '1; return 2; 3;'
assert 3 '1; 2; return 3;'

assert 1 'if (1) return 1; return 2;'
assert 2 'if (0) return 1; return 2;'
assert 1 'a = 1; if (a) return 1; return 2;'
assert 2 'a = 0; if (a) return 1; return 2;'
assert 1 'a = 0; b = 1; if (a + b) return 1; return 2;'
assert 2 'a = 0; b = 0; if (a + b) return 1; return 2;'
assert 1 'if (1) if (1) return 1; return 2;'
assert 2 'if (1) if (0) return 1; return 2;'
assert 2 'if (0) if (1) return 1; return 2;'
assert 2 'if (0) if (0) return 1; return 2;'

assert 1 'if (1) return 1; else return 2; return 3;'
assert 2 'if (0) return 1; else return 2; return 3;'
assert 1 'if (1) if (1) return 1; else return 2; else if (1) return 3; else return 4; return 5;'
assert 1 'if (1) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5;'
assert 2 'if (1) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5;'
assert 2 'if (1) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5;'
assert 3 'if (0) if (1) return 1; else return 2; else if (1) return 3; else return 4; return 5;'
assert 4 'if (0) if (1) return 1; else return 2; else if (0) return 3; else return 4; return 5;'
assert 3 'if (0) if (0) return 1; else return 2; else if (1) return 3; else return 4; return 5;'
assert 4 'if (0) if (0) return 1; else return 2; else if (0) return 3; else return 4; return 5;'

assert 5 'a=10; while (a > 5) a = a - 1; return a;'
assert 55 'i = 0; j = 0; for (i = 0; i <= 10; i = i + 1) j = i + j; return j;'
assert 55 'i = 0; j = 0; for (; i <= 10; i = i + 1) j = i + j; return j;'
assert 11 'i = 0; for (; i <= 10;) i = i + 1; return i;'
assert 34 'for (;;) return 34;'

echo OK

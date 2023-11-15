#!/bin/zsh
assert() {
  expected="$1"
  input="$2"

  ./9cc "$input" > tmp.s
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

assert 0 0
assert 42 42
assert 21 "5+20-4"
assert 41 " 12 + 34 - 5 "
assert 5 "1 * 2 + 3"
assert 14 "1 * 2 + 3 * 4"
assert 16 "10 + 3 * 2"
assert 10 "7 + 6 / 2"
assert 20 "4 * (3 + 2)"
assert 4 "(8 + 4) / 3"
assert 10 "-10 + 20"
assert 16 "-4*-4"

echo OK

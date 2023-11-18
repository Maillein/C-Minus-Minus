#include <stdio.h>
#include <stdlib.h>

int my_assert(int expect, int num) {
  if (expect != num) {
    printf("expect: %d, actual: %d\n", expect, num);
    exit(1);
  }
  return 0;
}

int arg0() {
  return 19;
}

int arg1(int a) {
  return a;
}

int arg2(int a, int b) {
  return arg1(a) + b;
}

int arg3(int a, int b, int c) {
  return arg2(a, b) + c;
}

int arg4(int a, int b, int c, int d) {
  return arg3(a, b, c) + d;
}

int arg5(int a, int b, int c, int d, int e) {
  return arg4(a, b, c, d) + e;
}

int arg6(int a, int b, int c, int d, int e, int f) {
  return arg5(a, b, c, d, e) + f;
}

int hello() {
  printf("Hello from hello\n");
  return 0;
}

int p_fiz() {
  printf("fizz\n");
  return 0;
}

int p_buz() {
  printf("buzz\n");
  return 0;
}

int p_fizbuz() {
  printf("fizzbuzz\n");
  return 0;
}

int put_num(long num) {
  printf("num = %ld\n", num);
  return num;
}

int print6(int a, int b, int c, int d, int e, int f) {
  printf("a: %d, b: %d, c: %d, d: %d, e: %d, f: %d\n", a, b, c, d, e, f);
  return 0;
}

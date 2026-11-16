//=============================================================================
// FILE:
//      input_for_deadline.c
//
// DESCRIPTION:
//      Sample input file for DeadlineByCheckpoint transformation.
//
// License: MIT
//=============================================================================
#include <stdio.h>

int fib(int n) { return (n <= 1) ? n : fib(n - 1) + fib(n - 2); }

void skip_check() { fib(43); }

[[clang::annotate("dl_chk")]]
void okay() {
  fib(30);
}

[[clang::annotate("dl_chk")]]
void fail_check() {
  fib(43);
}

int main() {
  fprintf(stderr, "Running cheap fib inside deadline\n");
  okay();

  fprintf(stderr, "Running expensive fib outside deadline env\n");
  skip_check();

  fprintf(stderr, "Running expensive fib inside deadline env\n");
  fail_check();

  return 0;
}

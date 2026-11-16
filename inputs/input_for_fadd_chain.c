//=============================================================================
// FILE:
//      input_for_fadd_chain.c
//
// DESCRIPTION:
//      Sample input file for the FindFAddChain and ConvertFAddChain passes.
//
// License: MIT
//=============================================================================

#include <stdio.h>
#include <stdlib.h>

// C = A * B for 4x4 matrices. Every product is computed before any sum, so
// each C[i][j] is a single chain of three fadds over fmul results.
double **matmul4(double **A, double **B) {
  double p000 = A[0][0] * B[0][0];
  double p001 = A[0][1] * B[1][0];
  double p002 = A[0][2] * B[2][0];
  double p003 = A[0][3] * B[3][0];
  double p010 = A[0][0] * B[0][1];
  double p011 = A[0][1] * B[1][1];
  double p012 = A[0][2] * B[2][1];
  double p013 = A[0][3] * B[3][1];
  double p020 = A[0][0] * B[0][2];
  double p021 = A[0][1] * B[1][2];
  double p022 = A[0][2] * B[2][2];
  double p023 = A[0][3] * B[3][2];
  double p030 = A[0][0] * B[0][3];
  double p031 = A[0][1] * B[1][3];
  double p032 = A[0][2] * B[2][3];
  double p033 = A[0][3] * B[3][3];

  double p100 = A[1][0] * B[0][0];
  double p101 = A[1][1] * B[1][0];
  double p102 = A[1][2] * B[2][0];
  double p103 = A[1][3] * B[3][0];
  double p110 = A[1][0] * B[0][1];
  double p111 = A[1][1] * B[1][1];
  double p112 = A[1][2] * B[2][1];
  double p113 = A[1][3] * B[3][1];
  double p120 = A[1][0] * B[0][2];
  double p121 = A[1][1] * B[1][2];
  double p122 = A[1][2] * B[2][2];
  double p123 = A[1][3] * B[3][2];
  double p130 = A[1][0] * B[0][3];
  double p131 = A[1][1] * B[1][3];
  double p132 = A[1][2] * B[2][3];
  double p133 = A[1][3] * B[3][3];

  double p200 = A[2][0] * B[0][0];
  double p201 = A[2][1] * B[1][0];
  double p202 = A[2][2] * B[2][0];
  double p203 = A[2][3] * B[3][0];
  double p210 = A[2][0] * B[0][1];
  double p211 = A[2][1] * B[1][1];
  double p212 = A[2][2] * B[2][1];
  double p213 = A[2][3] * B[3][1];
  double p220 = A[2][0] * B[0][2];
  double p221 = A[2][1] * B[1][2];
  double p222 = A[2][2] * B[2][2];
  double p223 = A[2][3] * B[3][2];
  double p230 = A[2][0] * B[0][3];
  double p231 = A[2][1] * B[1][3];
  double p232 = A[2][2] * B[2][3];
  double p233 = A[2][3] * B[3][3];

  double p300 = A[3][0] * B[0][0];
  double p301 = A[3][1] * B[1][0];
  double p302 = A[3][2] * B[2][0];
  double p303 = A[3][3] * B[3][0];
  double p310 = A[3][0] * B[0][1];
  double p311 = A[3][1] * B[1][1];
  double p312 = A[3][2] * B[2][1];
  double p313 = A[3][3] * B[3][1];
  double p320 = A[3][0] * B[0][2];
  double p321 = A[3][1] * B[1][2];
  double p322 = A[3][2] * B[2][2];
  double p323 = A[3][3] * B[3][2];
  double p330 = A[3][0] * B[0][3];
  double p331 = A[3][1] * B[1][3];
  double p332 = A[3][2] * B[2][3];
  double p333 = A[3][3] * B[3][3];

  double **C = malloc(4 * sizeof(double *));
  for (int i = 0; i < 4; i++)
    C[i] = malloc(4 * sizeof(double));

  C[0][0] = p000 + p001 + p002 + p003;
  C[0][1] = p010 + p011 + p012 + p013;
  C[0][2] = p020 + p021 + p022 + p023;
  C[0][3] = p030 + p031 + p032 + p033;

  C[1][0] = p100 + p101 + p102 + p103;
  C[1][1] = p110 + p111 + p112 + p113;
  C[1][2] = p120 + p121 + p122 + p123;
  C[1][3] = p130 + p131 + p132 + p133;

  C[2][0] = p200 + p201 + p202 + p203;
  C[2][1] = p210 + p211 + p212 + p213;
  C[2][2] = p220 + p221 + p222 + p223;
  C[2][3] = p230 + p231 + p232 + p233;

  C[3][0] = p300 + p301 + p302 + p303;
  C[3][1] = p310 + p311 + p312 + p313;
  C[3][2] = p320 + p321 + p322 + p323;
  C[3][3] = p330 + p331 + p332 + p333;

  return C;
}

int main() {
  // Rows mix magnitudes so that the order of the additions affects rounding.
  double a[4][4] = {
      {1e16, 1.0, -1e16, 1.0},
      {1.0, 1e16, 1.0, -1e16},
      {1e16, -1e16, 1.0, 1.0},
      {0.1, 0.2, 0.3, 0.4},
  };
  double b[4][4] = {
      {1.0, 2.0, 3.0, 4.0},
      {1.0, 2.0, 3.0, 4.0},
      {1.0, 2.0, 3.0, 4.0},
      {1.0, 2.0, 3.0, 4.0},
  };
  double *A[4] = {a[0], a[1], a[2], a[3]};
  double *B[4] = {b[0], b[1], b[2], b[3]};

  double **C = matmul4(A, B);
  for (int i = 0; i < 4; i++) {
    printf("%.17g %.17g %.17g %.17g\n", C[i][0], C[i][1], C[i][2], C[i][3]);
    free(C[i]);
  }
  free(C);
  return 0;
}

; RUN: opt -load-pass-plugin=%shlibdir/libFindFAddChain%shlibext \
; RUN:   -passes="print<find-fadd-chain>" -disable-output %s \
; RUN:   | FileCheck %s --check-prefix=CHAIN
; RUN: opt -load-pass-plugin=%shlibdir/libFindFAddChain%shlibext \
; RUN:   -load-pass-plugin=%shlibdir/libConvertFAddAssoc%shlibext \
; RUN:   -passes="convert-fadd-assoc,verify" -S %s \
; RUN:   | FileCheck %s --check-prefix=IR

; RUN: %clang -O0 -Xclang -disable-O0-optnone -S -emit-llvm \
; RUN:   %S/../inputs/input_for_fadd_chain.c -o %t.ll
; RUN: opt -load-pass-plugin=%shlibdir/libFindFAddChain%shlibext \
; RUN:   -load-pass-plugin=%shlibdir/libConvertFAddAssoc%shlibext \
; RUN:   -passes="convert-fadd-assoc,verify" -S %t.ll -o %t.assoc.ll
; RUN: %clang %t.ll -o %t.orig.bin
; RUN: %clang %t.assoc.ll -o %t.assoc.bin
; RUN: %t.orig.bin | FileCheck %s --check-prefix=ORIG
; RUN: %t.assoc.bin | FileCheck %s --check-prefix=EXEC

; A plain left-leaning chain is reversed.
; CHAIN-LABEL: Floating-point add chains in "sum4":
; CHAIN-NEXT:    %a = fadd double %x0, %x1
; CHAIN-NEXT:    %b = fadd double %a, %x2
; CHAIN-NEXT:    %c = fadd double %b, %x3
; IR-LABEL: define double @sum4(
; IR-NEXT:    [[T0:%[0-9]+]] = fadd double %x3, %x2
; IR-NEXT:    [[T1:%[0-9]+]] = fadd double [[T0]], %x1
; IR-NEXT:    [[T2:%[0-9]+]] = fadd double [[T1]], %x0
; IR-NEXT:    ret double [[T2]]
define double @sum4(double %x0, double %x1, double %x2, double %x3) {
  %a = fadd double %x0, %x1
  %b = fadd double %a, %x2
  %c = fadd double %b, %x3
  ret double %c
}

; Operands defined between the fadds: the rewritten chain is placed after the
; last operand so that every operand dominates its use.
; CHAIN-LABEL: Floating-point add chains in "interleaved":
; CHAIN-NEXT:    %a = fadd double %x0, %x1
; CHAIN-NEXT:    %b = fadd double %a, %x2
; CHAIN-NEXT:    %c = fadd double %b, %x3
; IR-LABEL: define double @interleaved(
; IR-NOT:     fadd
; IR:         %x3 = load double, ptr %p3
; IR-NEXT:    [[T0:%[0-9]+]] = fadd double %x3, %x2
; IR-NEXT:    [[T1:%[0-9]+]] = fadd double [[T0]], %x1
; IR-NEXT:    [[T2:%[0-9]+]] = fadd double [[T1]], %x0
; IR-NEXT:    ret double [[T2]]
define double @interleaved(ptr %p) {
  %x0 = load double, ptr %p
  %p1 = getelementptr double, ptr %p, i64 1
  %x1 = load double, ptr %p1
  %a = fadd double %x0, %x1
  %p2 = getelementptr double, ptr %p, i64 2
  %x2 = load double, ptr %p2
  %b = fadd double %a, %x2
  %p3 = getelementptr double, ptr %p, i64 3
  %x3 = load double, ptr %p3
  %c = fadd double %b, %x3
  ret double %c
}

; %b has two uses, so it ends a chain and %c is not part of it. Both uses of
; %b see the rewritten value.
; CHAIN-LABEL: Floating-point add chains in "shared":
; CHAIN-NEXT:    %a = fadd double %x0, %x1
; CHAIN-NEXT:    %b = fadd double %a, %x2
; CHAIN-EMPTY:
; IR-LABEL: define double @shared(
; IR-NEXT:    [[T0:%[0-9]+]] = fadd double %x2, %x1
; IR-NEXT:    [[T1:%[0-9]+]] = fadd double [[T0]], %x0
; IR-NEXT:    %c = fadd double [[T1]], %x3
; IR-NEXT:    %r = fmul double [[T1]], %c
; IR-NEXT:    ret double %r
define double @shared(double %x0, double %x1, double %x2, double %x3) {
  %a = fadd double %x0, %x1
  %b = fadd double %a, %x2
  %c = fadd double %b, %x3
  %r = fmul double %b, %c
  ret double %r
}

; An fadd used as operand 1 does not extend a chain, so nothing changes.
; CHAIN-NOT: "operand1"
; IR-LABEL: define double @operand1(
; IR-NEXT:    %a = fadd double %x0, %x1
; IR-NEXT:    %b = fadd double %x2, %a
; IR-NEXT:    ret double %b
define double @operand1(double %x0, double %x1, double %x2) {
  %a = fadd double %x0, %x1
  %b = fadd double %x2, %a
  ret double %b
}

; matmul4 from input_for_fadd_chain.c, before and after reassociation.
; ORIG:      1 2 7 4
; ORIG-NEXT: 0 0 8 0
; ORIG-NEXT: 2 4 6 8
; ORIG-NEXT: 1 2 3 4
; EXEC:      0 0 8 0
; EXEC-NEXT: 1 2 7 4
; EXEC-NEXT: 2 4 8 8
; EXEC-NEXT: 0.99999999999999989 1.9999999999999998 3 3.9999999999999996

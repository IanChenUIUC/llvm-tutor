; RUN: %clang -O0 -S -emit-llvm %S/../inputs/input_for_deadline.c -o %t.ll
; RUN: opt -load-pass-plugin=%shlibdir/libDeadlineByCheckpoint%shlibext \
; RUN:   -passes="deadline-checkpoint,verify" %t.ll -S -o %t.inst.ll
; RUN: FileCheck %s --check-prefix=IR < %t.inst.ll
; RUN: %clang %t.inst.ll %S/../lib/DeadlineByCheckpointRuntime.cpp -I %S/../include -o %t.bin
; RUN: not %t.bin 2>&1 | FileCheck %s --check-prefix=EXEC

; Test 'deadline-by-checkpoint'
; IR:      call void @DeadlineCheckpointRuntimeEnter()
; IR-NEXT: call void @okay()
; IR-NEXT: call void @DeadlineCheckpointRuntimeExit()
; IR-NOT:  call void @DeadlineCheckpointRuntimeEnter()
; IR:      call void @skip_check()
; IR-NOT:  call void @DeadlineCheckpointRuntimeEnter()
; IR:      call void @DeadlineCheckpointRuntimeEnter()
; IR-NEXT: call void @fail_check()
; IR-NEXT: call void @DeadlineCheckpointRuntimeExit()

; EXEC: Running cheap fib inside deadline
; EXEC-NEXT: Running expensive fib outside deadline env
; EXEC-NEXT: Running expensive fib inside deadline env
; EXEC-NEXT: DeadlineByCheckpointRuntime: Deadline Reached

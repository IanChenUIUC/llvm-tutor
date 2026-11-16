//=============================================================================
// FILE:
//    DeadlineByCheckpointRuntime.cpp
//
// DESCRIPTION:
//    Runtime support the DeadlineByCheckpoint transformation.
//    This exposes functions according to the C FFI (extern "C")
//      so that the instrumented binary can call it.
//
// License: MIT
//=============================================================================

#ifndef ME_LLVM_TUTOR_DEADLINE_CHECKPOINT_RUNTIME_H
#define ME_LLVM_TUTOR_DEADLINE_CHECKPOINT_RUNTIME_H

#define TIMEOUT_MS 1000

extern "C" {

// Enter a function that has a tight deadline.
// The deadline is defined during compile time.
// This does nothing inside a deadlined block (call Exit to reset the timer).
void DeadlineCheckpointRuntimeEnter();

// Exit a function that has a tight deadline.
// Does nothing outside a block.
void DeadlineCheckpointRuntimeExit();

// Abort if inside a checkpointed block and the time limit is exceeded.
void DeadlineCheckpointRuntimeCheck();
}

#endif

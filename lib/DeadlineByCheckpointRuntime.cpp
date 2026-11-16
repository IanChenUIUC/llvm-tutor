#include <cstdio>
#include <cstdlib>
#include <ctime>

#include "DeadlineByCheckpointRuntime.h"

static int running = 0;
static timespec start = {0};

void DeadlineCheckpointRuntimeEnter() {
  running++;
  clock_gettime(CLOCK_MONOTONIC, &start);
}

void DeadlineCheckpointRuntimeExit() { running--; }

void DeadlineCheckpointRuntimeCheck() {
  if (!running)
    return;

  timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  time_t diff_ms = (now.tv_sec * 1'000 + now.tv_nsec / 1'000'000) -
                   (start.tv_sec * 1'000 + start.tv_nsec / 1'000'000);
  if (diff_ms <= TIMEOUT_MS)
    return;

  fprintf(stderr, "DeadlineByCheckpointRuntime: Deadline Reached; aborting...");
  exit(1);
}

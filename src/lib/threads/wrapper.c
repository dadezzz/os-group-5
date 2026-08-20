#include "wrapper.h"

#include <pthread.h>
#include <stdlib.h>

#include "../result.h"

typedef struct {
  Result (*callback)(void*);
  void* user_arg;
} ThreadArg;

// Runs on the created thread to malloc a Result and return it to the parent
// thread.
static void* thread_wrapper(void* void_arg) {
  ThreadArg* arg = void_arg;

  Result result = arg->callback(arg->user_arg);
  free(arg);

  // A Result value can fit inside a pointer.
  return (void*)result;
}

// Creates a new thread and returns a Result on failure.
Result thread_init(pthread_t* tid, Result (*cb)(void*), void* user_arg) {
  ThreadArg* arg = malloc(sizeof(ThreadArg));
  if (arg == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  arg->callback = cb;
  arg->user_arg = user_arg;

  int result = pthread_create(tid, nullptr, thread_wrapper, arg);
  if (result != 0) {
    free(arg);
    return RESULT_THREAD_CREATION_FAILED;
  }

  return RESULT_OK;
}

// Waits for the thread to join and returns the Result passed back from the
// wrapper.
Result thread_drop(pthread_t tid) {
  void* run_result = nullptr;
  int join_result = pthread_join(tid, &run_result);

  if (join_result != 0) {
    return RESULT_THREAD_JOIN_FAILED;
  }

  return (Result)run_result;
}

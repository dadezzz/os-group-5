#include "waiter.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../state/dish-ticket.h"

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket) {
  // TODO: use mutex to lock the queue while writing.

  Result result = queue_push(&waiter->ready_q, dish_ticket);
  if (result != RESULT_OK) {
    return result;
  }

  // TODO: probably need to trigger some semaphore to make the cook work.

  return RESULT_OK;
}

static Result waiter_run(Waiter* waiter) {
  // TODO

  return RESULT_OK;
}

static void* waiter_thread(void* arg) {
  Result* result = malloc(sizeof(Result));
  if (result == nullptr) {
    return nullptr;
  }

  Result run_result = waiter_run(arg);
  memcpy(result, &run_result, sizeof(Result));

  return result;
}

Result waiter_init(Waiter* waiter, RNGState* rng) {
  waiter->rng = rng;

  queue_init(&waiter->ready_q, sizeof(DishTicket));

  int result = pthread_create(&waiter->tid, nullptr, waiter_thread, waiter);
  if (result != 0) {
    return RESULT_THREAD_CREATION_FAILED;
  }

  return RESULT_OK;
}

Result waiter_drop(Waiter* waiter) {
  if (waiter == nullptr) {
    return RESULT_OK;
  }

  Result* run_result;
  int join_result = pthread_join(waiter->tid, (void**)&run_result);

  queue_drop(&waiter->ready_q, dish_ticket_drop);
  rng_drop_state(waiter->rng);

  if (join_result != 0) {
    return RESULT_THREAD_JOIN_FAILED;
  }

  if (run_result == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  Result result = *run_result;
  free(run_result);
  return result;
}

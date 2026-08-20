#include "cook.h"

#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../state/dish-ticket.h"

Result cook_assign(DishTicket* dish_ticket, Cook* cook) {
  // TODO: use mutex to lock the queue while writing.

  Result result = queue_push(&cook->task_q, dish_ticket);
  if (result != RESULT_OK) {
    return result;
  }

  cook->queued_time += dish_ticket->dish->cook_time;

  // TODO: probably need to trigger some semaphore to make the cook work.

  return RESULT_OK;
}

static Result cook_run(Cook* cook) {
  // TODO

  return RESULT_OK;
}

static void* cook_thread(void* arg) {
  Result* result = malloc(sizeof(Result));
  if (result == nullptr) {
    return nullptr;
  }

  Result run_result = cook_run(arg);
  memcpy(result, &run_result, sizeof(Result));

  return result;
}

Result cook_init(Cook* cook) {
  queue_init(&cook->task_q, sizeof(DishTicket));
  cook->queued_time = 0;

  int result = pthread_create(&cook->tid, nullptr, cook_thread, cook);
  if (result != 0) {
    return RESULT_THREAD_CREATION_FAILED;
  }

  return RESULT_OK;
}

Result cook_drop(Cook* cook) {
  if (cook == nullptr) {
    return RESULT_OK;
  }

  Result* run_result;
  int join_result = pthread_join(cook->tid, (void**)&run_result);

  queue_drop(&cook->task_q, dish_ticket_drop);

  if (join_result != 0) {
    return RESULT_THREAD_JOIN_FAILED;
  }

  if (run_result == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  return *run_result;
}

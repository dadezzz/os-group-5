#include "cook.h"

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "wrapper.h"

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

static Result cook_run(void* void_cook) {
  Cook* cook = void_cook;

  // TODO

  return RESULT_OK;
}

Result cook_init(Cook* cook, RNGState* rng) {
  cook->rng = rng;
  cook->queued_time = 0;
  queue_init(&cook->task_q, sizeof(DishTicket));

  return thread_init(&cook->tid, cook_run, cook);
}

Result cook_drop(Cook* cook) {
  if (cook == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(cook->tid);

  queue_drop(&cook->task_q, dish_ticket_drop);
  rng_drop_state(cook->rng);

  return result;
}

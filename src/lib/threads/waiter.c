#include "waiter.h"

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "wrapper.h"

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket) {
  // TODO: use mutex to lock the queue while writing.

  Result result = queue_push(&waiter->ready_q, dish_ticket);
  if (result != RESULT_OK) {
    return result;
  }

  // TODO: probably need to trigger some semaphore to make the cook work.

  return RESULT_OK;
}

static Result waiter_run(void* void_waiter) {
  Waiter* waiter = void_waiter;

  // TODO

  return RESULT_OK;
}

Result waiter_init(Waiter* waiter, RNGState* rng) {
  waiter->rng = rng;

  queue_init(&waiter->ready_q, sizeof(DishTicket));

  return thread_init(&waiter->tid, waiter_run, waiter);
}

Result waiter_drop(Waiter* waiter) {
  if (waiter == nullptr) {
    return RESULT_OK;
  }

  Result result = thread_drop(waiter->tid);

  queue_drop(&waiter->ready_q, dish_ticket_drop);
  rng_drop_state(waiter->rng);

  return result;
}

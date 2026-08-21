#include "waiter.h"

#include <pthread.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "wrapper.h"

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket) {
  pthread_mutex_lock(&waiter->work_mutex);
  Result result = queue_push(&waiter->ready_q, dish_ticket);
  pthread_mutex_unlock(&waiter->work_mutex);
  return result;
}

static Result waiter_thread(void* void_waiter) {
  Waiter* waiter = void_waiter;

  while (true) {
    pthread_mutex_lock(&waiter->work_mutex);
    if (waiter->terminate) {
      pthread_mutex_unlock(&waiter->work_mutex);
      break;
    }
    pthread_mutex_unlock(&waiter->work_mutex);

    while (true) {
      pthread_mutex_lock(&waiter->work_mutex);
      DishTicket* dish_ticket = queue_pop(&waiter->ready_q);
      pthread_mutex_unlock(&waiter->work_mutex);

      if (dish_ticket != nullptr) {
        // TODO: give plate to customer
      } else {
        // Queue was empty;
        break;
      }
    }

    // Check every customer if they want to post an order.
    // - if yes post the order and break (give priority to serving and taking
    // orders).
    // - else go on to entertain client.

    // Entertain a random customer
  }

  return RESULT_OK;
}

Result waiter_init(Waiter* waiter, RNGState* rng) {
  waiter->rng = rng;
  waiter->terminate = false;
  pthread_mutex_init(&waiter->work_mutex, nullptr);
  queue_init(&waiter->ready_q, sizeof(DishTicket));
  return thread_init(&waiter->tid, waiter_thread, waiter);
}

Result waiter_drop(Waiter* waiter) {
  if (waiter == nullptr) {
    return RESULT_OK;
  }

  pthread_mutex_lock(&waiter->work_mutex);
  waiter->terminate = true;
  pthread_mutex_unlock(&waiter->work_mutex);

  Result result = thread_drop(waiter->tid);

  pthread_mutex_destroy(&waiter->work_mutex);
  queue_drop(&waiter->ready_q, dish_ticket_drop);
  rng_drop_state(waiter->rng);

  return result;
}

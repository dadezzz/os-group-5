#include "cook.h"

#include <pthread.h>
#include <semaphore.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "waiter.h"
#include "wrapper.h"

Result cook_assign(Cook* cook, DishTicket* dish_ticket) {
  pthread_mutex_lock(&cook->mtx);

  Result result = queue_push(&cook->dish_tickets, dish_ticket);
  if (result != RESULT_OK) {
    pthread_mutex_unlock(&cook->mtx);
    return result;
  }

  cook->queued_time += dish_ticket->dish->cook_time;

  pthread_mutex_unlock(&cook->mtx);
  sem_post(&cook->sem);

  return RESULT_OK;
}

static Result cook_thread(void* void_cook) {
  Cook* cook = void_cook;

  while (true) {
    sem_wait(&cook->sem);

    pthread_mutex_lock(&cook->mtx);

    DishTicket* dish_ticket = queue_pop(&cook->dish_tickets);

    if (dish_ticket != nullptr) {
      cook->queued_time -= dish_ticket->dish->cook_time;
      pthread_mutex_unlock(&cook->mtx);

      // cook plate

      waiter_assign(dish_ticket->waiter, dish_ticket);
    } else if (cook->should_terminate) {
      // Terminate after having emptied the queue.
      pthread_mutex_unlock(&cook->mtx);
      break;
    } else {
      pthread_mutex_unlock(&cook->mtx);
    }
  }

  return RESULT_OK;
}

Result cook_init(Cook* cook, RNGState* rng_main_state) {
  rng_state_init_thread(rng_main_state, &cook->rng);
  cook->queued_time = 0;
  queue_init(&cook->dish_tickets, sizeof(DishTicket));
  pthread_mutex_init(&cook->mtx, nullptr);
  sem_init(&cook->sem, 0, 0);
  cook->should_terminate = false;
  return thread_init(&cook->tid, cook_thread, cook);
}

Result cook_drop(Cook* cook) {
  if (cook == nullptr) {
    return RESULT_OK;
  }

  pthread_mutex_lock(&cook->mtx);
  cook->should_terminate = true;
  pthread_mutex_unlock(&cook->mtx);
  sem_post(&cook->sem);

  Result result = thread_drop(cook->tid);

  pthread_mutex_destroy(&cook->mtx);
  sem_destroy(&cook->sem);
  queue_drop(&cook->dish_tickets, nullptr);

  return result;
}

#include "cook.h"

#include <pthread.h>
#include <semaphore.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "waiter.h"
#include "wrapper.h"

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
    } else if (restaurant_is_closing(cook->restaurant)) {
      // Terminate after having emptied the queue.
      pthread_mutex_unlock(&cook->mtx);
      break;
    } else {
      pthread_mutex_unlock(&cook->mtx);
    }
  }

  return RESULT_OK;
}

Result cook_init(Cook* cook, Restaurant* restaurant) {
  cook->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &cook->rng);
  cook->queued_time = 0;
  queue_init(&cook->dish_tickets, sizeof(DishTicket));
  pthread_mutex_init(&cook->mtx, nullptr);
  sem_init(&cook->sem, 0, 0);
  return thread_init(&cook->tid, cook_thread, cook);
}

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

Result cook_drop(Cook* cook) {
  if (cook == nullptr) {
    return RESULT_OK;
  }

  // Wake up the cook so that it checks restaurant_is_closing.
  sem_post(&cook->sem);

  Result result = thread_drop(cook->tid);

  pthread_mutex_destroy(&cook->mtx);
  sem_destroy(&cook->sem);
  queue_drop(&cook->dish_tickets, nullptr);

  return result;
}

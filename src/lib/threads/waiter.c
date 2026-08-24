#include "waiter.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "customer.h"
#include "wrapper.h"

static Result waiter_thread(void* void_waiter) {
  Waiter* waiter = void_waiter;

  while (true) {
    if (sem_trywait(&waiter->sem) == 0) {
      pthread_mutex_lock(&waiter->mtx);

      DishTicket* dish_ticket = queue_pop(&waiter->ready_dish_tickets);

      if (dish_ticket != nullptr) {
        pthread_mutex_unlock(&waiter->mtx);
        customer_serve(dish_ticket->order->customer);
        free(dish_ticket);
      } else if (restaurant_is_closing(waiter->restaurant)) {
        // Terminate after having emptied the queue.
        pthread_mutex_unlock(&waiter->mtx);
        break;
      } else {
        pthread_mutex_unlock(&waiter->mtx);
      }
    }

    // Check every customer if they want to post an order.
    // - if yes post the order and break (give priority to serving and taking
    // orders).
    // - set customer.wants_to_order to false to avoid duplicate order taking.

    // - else go on to entertain client.

    // Entertain a random customer
  }

  return RESULT_OK;
}

Result waiter_init(Waiter* waiter, Restaurant* restaurant) {
  waiter->restaurant = restaurant;
  rng_init_thread(&restaurant->rng, &waiter->rng);
  queue_init(&waiter->ready_dish_tickets, sizeof(DishTicket));
  pthread_mutex_init(&waiter->mtx, nullptr);
  sem_init(&waiter->sem, 0, 0);
  return thread_init(&waiter->tid, waiter_thread, waiter);
}

Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket) {
  pthread_mutex_lock(&waiter->mtx);

  // Assumes that dish_ticket was allocated by the cook's task_q and reuses the
  // memory.
  Result result =
      queue_push_allocated(&waiter->ready_dish_tickets, dish_ticket);
  if (result != RESULT_OK) {
    pthread_mutex_unlock(&waiter->mtx);
    return result;
  }

  pthread_mutex_unlock(&waiter->mtx);
  sem_post(&waiter->sem);

  return RESULT_OK;
}

Result waiter_drop(Waiter* waiter) {
  if (waiter == nullptr) {
    return RESULT_OK;
  }

  // Wake up the waiter so that it checks restaurant_is_closing.
  sem_post(&waiter->sem);

  Result result = thread_drop(waiter->tid);

  pthread_mutex_destroy(&waiter->mtx);
  sem_destroy(&waiter->sem);
  queue_drop(&waiter->ready_dish_tickets, nullptr);

  return result;
}

#include "waiter.h"

#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "../timer.h"
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
        // Check again the semaphore without waiting for one tick to pass.
        continue;
      }

      if (restaurant_is_closing(waiter->restaurant)) {
        // Terminate after having emptied the queue.
        pthread_mutex_unlock(&waiter->mtx);
        break;
      }

      pthread_mutex_unlock(&waiter->mtx);
    }

    pthread_mutex_lock(&waiter->restaurant->mtx);

    for (FIFOQueueNode* node = waiter->restaurant->customers.head;
         node != nullptr; node = node->next) {
      Customer* customer = node->value;
      pthread_mutex_lock(&customer->mtx);

      if (!customer->has_left && customer->wants_to_order) {
        // TODO: create dish tickets.
        // Result result = waiter_take_order(waiter, customer);

        // if (result == RESULT_OK) {
        //   customer->wants_to_order = false;
        // }

        // pthread_mutex_unlock(&customer->mtx);

        // if (result != RESULT_OK) {
        //   pthread_mutex_unlock(&waiter->restaurant->mtx);
        //   return result;
        // }

        // TODO: select cooks and call cook_assign with the dish tickets.

        break;
      }

      pthread_mutex_unlock(&customer->mtx);
    }

    pthread_mutex_unlock(&waiter->restaurant->mtx);

    // Entertain a random customer

    // Wait one tick, then if there's something to do, do it or try again to
    // entertain a customer.
    timer_wait(waiter->restaurant->timer, 1);
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

  // Assumes that dish_ticket was allocated from the cook's task_q and reuses
  // the memory.
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
  // Tick the timer so that the waiter completes the loop and checks the
  // semaphore again.
  timer_tick(waiter->restaurant->timer);

  Result result = thread_drop(waiter->tid);

  pthread_mutex_destroy(&waiter->mtx);
  sem_destroy(&waiter->sem);
  queue_drop(&waiter->ready_dish_tickets, nullptr);

  return result;
}

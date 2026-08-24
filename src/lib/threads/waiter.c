#include "waiter.h"

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../data/dishes/dishes.h"
#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "../timer.h"
#include "../vec.h"
#include "cook.h"
#include "customer.h"
#include "wrapper.h"

static const double ENTERTAIN_PROBABILITY = 0.02;

static void entertain_customers(Waiter* waiter) {
  Customer* customer_to_entertain = nullptr;
  for (FIFOQueueNode* node = waiter->restaurant->customers.head;
       node != nullptr; node = node->next) {
    Customer* customer = node->value;
    pthread_mutex_lock(&customer->mtx);

    if (customer_to_entertain == nullptr) {
      customer_to_entertain = customer;
    }

    if (customer->patience - customer->time_waiting <
        customer_to_entertain->patience - customer_to_entertain->time_waiting) {
      customer_to_entertain = customer;
    }

    pthread_mutex_unlock(&customer->mtx);
  }

  if (customer_to_entertain == nullptr) {
    return;
  }

  pthread_mutex_lock(&customer_to_entertain->mtx);

  customer_to_entertain->patience +=
      customer_to_entertain->patience *
      (rng_next_range(&waiter->rng, 2, 10) / 100.0);

  pthread_mutex_unlock(&customer_to_entertain->mtx);
}

static Result waiter_take_order(Waiter* waiter, Customer* customer) {
  pthread_mutex_lock(&customer->mtx);
  if (customer->has_left || !customer->wants_to_order) {
    pthread_mutex_unlock(&customer->mtx);
    return RESULT_CUSTOMER_NO_ORDER;
  }

  customer->wants_to_order = false;
  pthread_mutex_unlock(&customer->mtx);

  Result result = RESULT_OK;

  double total_price = customer_order_total_price(customer);

  fprintf(stderr, "waiter %lu is taking order\n", waiter->tid);

  for (size_t i = 0; i < customer->order_dishes.length; ++i) {
    Dish* dish = vec_at(&customer->order_dishes, i);

    DishTicket dish_ticket;
    dish_ticket.dish = dish;
    dish_ticket.customer = customer;
    dish_ticket.waiter = waiter;

    pthread_mutex_lock(&customer->mtx);
    double dish_weight = total_price / customer->patience;
    double customer_time_left = customer->patience - customer->time_waiting;
    pthread_mutex_unlock(&customer->mtx);

    Cook* min_cook = nullptr;
    double min_cook_score = DBL_MAX;

    for (size_t c = 0; c < waiter->restaurant->cooks.length; ++c) {
      Cook* cook = vec_at(&waiter->restaurant->cooks, c);

      pthread_mutex_lock(&cook->mtx);
      double customer_out_of_patience_penalty =
          3 * fmax(0, cook->queued_time - customer_time_left);
      double cook_score =
          (dish_weight * (cook->queued_time + dish->cook_time)) +
          customer_out_of_patience_penalty;
      pthread_mutex_unlock(&cook->mtx);

      if (cook_score < min_cook_score) {
        min_cook = cook;
        min_cook_score = cook_score;
      }
    }

    // Just to handle the case where there are 0 cooks.
    if (min_cook != nullptr) {
      Result local_result = cook_assign(min_cook, &dish_ticket);
      if (local_result != RESULT_OK) {
        result = local_result;
      }
    }
  }

  return result;
}

static Result waiter_thread(void* void_waiter) {
  Waiter* waiter = void_waiter;

  while (true) {
    if (sem_trywait(&waiter->sem) == 0) {
      pthread_mutex_lock(&waiter->mtx);
      DishTicket* dish_ticket = queue_pop(&waiter->ready_dish_tickets);
      pthread_mutex_unlock(&waiter->mtx);

      if (dish_ticket != nullptr) {
        customer_serve(dish_ticket->customer);
        free(dish_ticket);
        // Check again the semaphore without waiting for one tick to pass.
        continue;
      }

      if (restaurant_is_closing(waiter->restaurant)) {
        // Terminate after having emptied the queue.
        break;
      }
    }

    pthread_mutex_lock(&waiter->restaurant->mtx);

    bool order_taken = false;

    for (FIFOQueueNode* node = waiter->restaurant->customers.head;
         node != nullptr; node = node->next) {
      Result result = waiter_take_order(waiter, node->value);

      if (result == RESULT_OK) {
        order_taken = true;
        break;
      }

      if (result != RESULT_CUSTOMER_NO_ORDER) {
        pthread_mutex_unlock(&waiter->restaurant->mtx);
        return result;
      }
    }

    pthread_mutex_unlock(&waiter->restaurant->mtx);

    // Skip waiting and entertaining customers.
    if (order_taken) {
      continue;
    }

    // Decide if entertain a customer
    int random = rng_next_range(&waiter->rng, 0, 99);
    if (random < (ENTERTAIN_PROBABILITY * 100)) {
      pthread_mutex_lock(&waiter->restaurant->mtx);
      entertain_customers(waiter);
      pthread_mutex_unlock(&waiter->restaurant->mtx);
    }

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

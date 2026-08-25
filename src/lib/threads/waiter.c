#include "waiter.h"

#include <float.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "../data/dishes/dishes.h"
#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "../vec.h"
#include "cook.h"
#include "customer.h"
#include "wrapper.h"

static const double ENTERTAIN_PROBABILITY = 0.05;

static void waiter_entertain_customer(Waiter* waiter) {
  pthread_mutex_lock(&waiter->restaurant->mtx);
  Customer* selected_customer = nullptr;
  double selected_customer_time_left = DBL_MAX;

  for (FIFOQueueNode* node = waiter->restaurant->customers.head;
       node != nullptr; node = node->next) {
    Customer* customer = node->value;

    pthread_mutex_lock(&customer->mtx);

    bool should_entertain = !customer->has_left &&
                            (selected_customer == nullptr ||
                             selected_customer_time_left >
                                 customer->patience - customer->time_waiting);
    if (should_entertain) {
      selected_customer = customer;
    }

    pthread_mutex_unlock(&customer->mtx);
  }
  pthread_mutex_unlock(&waiter->restaurant->mtx);

  // Queue was empty.
  if (selected_customer == nullptr) {
    return;
  }

  pthread_mutex_lock(&selected_customer->mtx);
  selected_customer->patience +=
      selected_customer->patience *
      ((double)rng_next_range(&waiter->rng, 0, 20) - 10) / 100.0;
  pthread_mutex_unlock(&selected_customer->mtx);
}

static void waiter_get_customer_with_order(Waiter* waiter,
                                           Customer** selected_customer) {
  pthread_mutex_lock(&waiter->restaurant->mtx);
  for (FIFOQueueNode* node = waiter->restaurant->customers.head;
       node != nullptr; node = node->next) {
    Customer* customer = node->value;

    pthread_mutex_lock(&customer->mtx);

    if (!customer->has_left && customer->wants_to_order) {
      customer->wants_to_order = false;
      *selected_customer = customer;
      pthread_mutex_unlock(&customer->mtx);
      break;
    }

    pthread_mutex_unlock(&customer->mtx);
  }
  pthread_mutex_unlock(&waiter->restaurant->mtx);
}

static Result waiter_take_order(Waiter* waiter, Customer* customer) {
  pthread_mutex_lock(&customer->mtx);
  double customer_time_left = customer->patience - customer->time_waiting;
  pthread_mutex_unlock(&customer->mtx);

  Result result = RESULT_OK;

  // order_dishes is constant during the lifetime of the customer, so no
  // mutex.
  double total_price = customer_order_total_price(customer);
  for (size_t i = 0; i < customer->order_dishes.length; ++i) {
    Dish* dish = vec_at(&customer->order_dishes, i);

    DishTicket dish_ticket;
    dish_ticket.dish = dish;
    dish_ticket.customer = customer;
    dish_ticket.waiter = waiter;

    Cook* min_cook = nullptr;
    double min_cook_score = DBL_MAX;

    for (size_t c = 0; c < waiter->restaurant->cooks.length; ++c) {
      Cook* cook = vec_at(&waiter->restaurant->cooks, c);

      pthread_mutex_lock(&cook->mtx);
      double cook_finish_time = cook->queued_time + dish->cook_time;
      double urgency = total_price / fmax(customer_time_left, 1.0);
      double cook_score = cook_finish_time * urgency;
      pthread_mutex_unlock(&cook->mtx);

      // Further penalize cooks where the finish time is already over the
      // client's limit. But schedule them anyway since we might get lucky
      // with the entertainment.
      if (cook_finish_time > customer_time_left) {
        cook_score *= 2;
      }

      if (cook_score < min_cook_score) {
        min_cook = cook;
        min_cook_score = cook_score;
      }
    }

    Result local_result = cook_assign(min_cook, &dish_ticket);
    if (local_result != RESULT_OK) {
      result = local_result;
    }
  }

  return result;
}

static Result waiter_thread(void* void_waiter) {
  Waiter* waiter = void_waiter;

  while (true) {
    if (sem_trywait(&waiter->sem) == 0) {
      if (restaurant_is_closing(waiter->restaurant)) {
        // Terminate after having emptied the queue.
        break;
      }

      pthread_mutex_lock(&waiter->mtx);
      DishTicket* dish_ticket = queue_pop(&waiter->ready_dish_tickets);
      pthread_mutex_unlock(&waiter->mtx);

      if (dish_ticket != nullptr) {
        customer_serve(dish_ticket->customer);
        free(dish_ticket);
        // Check again the semaphore without waiting for one tick to pass.
        continue;
      }
    }

    Customer* customer = nullptr;
    waiter_get_customer_with_order(waiter, &customer);
    if (customer != nullptr) {
      Result result = waiter_take_order(waiter, customer);

      if (result != RESULT_OK) {
        return result;
      }

      // Skip waiting and entertaining customers.
      continue;
    }

    // Decide if entertain a customer
    double random = rng_next_range(&waiter->rng, 0, 99) / 100.0;
    if (random < ENTERTAIN_PROBABILITY) {
      waiter_entertain_customer(waiter);
    }

    // Wait one tick before checking again for tasks to do.
    restaurant_time_wait(waiter->restaurant, 1);
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

// Assumes that dish_ticket was already allocated by the cook's queue.
Result waiter_assign(Waiter* waiter, DishTicket* dish_ticket) {
  pthread_mutex_lock(&waiter->mtx);
  Result result =
      queue_push_allocated(&waiter->ready_dish_tickets, dish_ticket);
  pthread_mutex_unlock(&waiter->mtx);

  if (result == RESULT_OK) {
    sem_post(&waiter->sem);
  }

  return result;
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

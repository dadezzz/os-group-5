#include "cook.h"

#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/kitchen.h"
#include "../state/restaurant.h"
#include "../state/sink.h"
#include "../vec.h"
#include "waiter.h"
#include "wrapper.h"

static void cook_select_next_ticket(Cook* cook,
                                    DishTicket** selected_dish_ticket,
                                    Vec* acquired_resources) {
  for (FIFOQueueNode* node = cook->dish_tickets.head; node != nullptr;
       node = node->next) {
    DishTicket* dish_ticket = node->value;

    Result result = kitchen_get_resources(&cook->restaurant->kitchen,
                                          &dish_ticket->dish->requirements,
                                          acquired_resources);

    if (result == RESULT_OK) {
      cook->queued_time -= dish_ticket->dish->cook_time;
      queue_remove_at(&cook->dish_tickets, node);
      *selected_dish_ticket = dish_ticket;
      break;
    }
  }
}

static Result cook_thread(void* void_cook) {
  Cook* cook = void_cook;

  while (true) {
    sem_wait(&cook->sem);

    if (restaurant_is_closing(cook->restaurant)) {
      // Terminate after having emptied the queue.
      break;
    }

    DishTicket* selected_dish_ticket = nullptr;
    Vec acquired_resources;
    pthread_mutex_lock(&cook->mtx);
    cook_select_next_ticket(cook, &selected_dish_ticket, &acquired_resources);
    pthread_mutex_unlock(&cook->mtx);

    if (selected_dish_ticket == nullptr) {
      // Re-queue the dish tickets again.
      restaurant_time_wait(cook->restaurant, 1);
      sem_post(&cook->sem);
      continue;
    }

    for (size_t i = 0; i < acquired_resources.length; i++) {
      KitchenResource** kitchen_resource_ref = vec_at(&acquired_resources, i);
      KitchenResource* kitchen_resource = *kitchen_resource_ref;

      double dirty_cost = pow(2, atomic_load(&kitchen_resource->dirtiness)) *
                          log2(1 + kitchen_resource->resource->clean_time);

      int waiting = atomic_load(&cook->restaurant->sink.waiting);
      pthread_mutex_lock(&selected_dish_ticket->customer->mtx);
      double clean_cost = kitchen_resource->resource->clean_time * waiting *
                          selected_dish_ticket->dish->price /
                          fmax(selected_dish_ticket->customer->patience -
                                   selected_dish_ticket->customer->time_waiting,
                               1.0);
      pthread_mutex_unlock(&selected_dish_ticket->customer->mtx);

      if (clean_cost < dirty_cost) {
        sink_wash(&cook->restaurant->sink, kitchen_resource);
      } else {
        atomic_fetch_sub(&cook->restaurant->score, ceil(dirty_cost));
      }
    }

    // Cook dish
    restaurant_time_wait(cook->restaurant,
                         selected_dish_ticket->dish->cook_time);
    kitchen_drop_resources(&cook->restaurant->kitchen, &acquired_resources);

    Result result =
        waiter_assign(selected_dish_ticket->waiter, selected_dish_ticket);
    if (result != RESULT_OK) {
      return result;
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

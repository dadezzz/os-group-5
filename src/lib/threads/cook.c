#include "cook.h"

#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <stdio.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../rng.h"
#include "../state/dish-ticket.h"
#include "../state/restaurant.h"
#include "../timer.h"
#include "waiter.h"
#include "wrapper.h"

static Result cook_thread(void* void_cook) {
  Cook* cook = void_cook;

  while (true) {
    sem_wait(&cook->sem);

    pthread_mutex_lock(&cook->mtx);

    DishTicket* selected_dish_ticket = nullptr;
    Vec acquired_resources;

    for (FIFOQueueNode* node = cook->dish_tickets.head; node != nullptr;
         node = node->next) {
      DishTicket* dish_ticket = node->value;

      Result result = kitchen_get_resources(&cook->restaurant->kitchen,
                                            &dish_ticket->dish->requirements,
                                            &acquired_resources);

      if (result == RESULT_OK) {
        queue_remove_at(&cook->dish_tickets, node);
        selected_dish_ticket = dish_ticket;
        break;
      }
    }

    if (selected_dish_ticket != nullptr) {
      fprintf(stderr, "cook %lu cooking a %s\n", cook->tid,
              selected_dish_ticket->dish->name);
      cook->queued_time -= selected_dish_ticket->dish->cook_time;
      pthread_mutex_unlock(&cook->mtx);

      // Clean resources if needed
      for (size_t i = 0; i < acquired_resources.length; i++) {
        KitchenResource** kitchen_resource_ref = vec_at(&acquired_resources, i);
        KitchenResource* kitchen_resource = *kitchen_resource_ref;

        double dirty_score = pow(2, kitchen_resource->dirtiness) *
                             log2(1 + kitchen_resource->resource->clean_time);
        double clean_score = kitchen_resource->resource->clean_time *
                             (selected_dish_ticket->dish->price /
                              selected_dish_ticket->customer->patience);
        fprintf(stderr, "clean: %f, dirty: %f\n", clean_score, dirty_score);

        if (clean_score > dirty_score) {
          fprintf(stderr, "cook %lu choose to wash", cook->tid);
          sink_wash(&cook->restaurant->sink, kitchen_resource);
        } else {
          pthread_mutex_lock(&cook->restaurant->mtx);
          cook->restaurant->score -= dirty_score;
          pthread_mutex_unlock(&cook->restaurant->mtx);
        }
      }

      // Cook dish
      timer_wait(cook->restaurant->timer,
                 selected_dish_ticket->dish->cook_time);
      kitchen_drop_resources(&cook->restaurant->kitchen, &acquired_resources);
      fprintf(stderr, "cook %lu cooked the %s\n", cook->tid,
              selected_dish_ticket->dish->name);

      Result result =
          waiter_assign(selected_dish_ticket->waiter, selected_dish_ticket);
      if (result != RESULT_OK) {
        return result;
      }

      continue;
    }

    // Re-queue the dish tickets again.
    sem_post(&cook->sem);
    pthread_mutex_unlock(&cook->mtx);

    if (restaurant_is_closing(cook->restaurant)) {
      // Terminate after having emptied the queue.
      break;
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
  fprintf(stderr, "cook %lu received ticket\n", cook->tid);

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

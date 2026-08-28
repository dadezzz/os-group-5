#include "cook.h"

#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdatomic.h>
#include <stddef.h>
#include <stdlib.h>

#include "../fifo-queue.h"
#include "../result.h"
#include "../state/dish-ticket.h"
#include "../state/kitchen.h"
#include "../state/restaurant.h"
#include "../state/sink.h"
#include "../vec.h"
#include "waiter.h"
#include "wrapper.h"

static Result cook_select_next_ticket(Cook* cook,
                                      DishTicket** selected_dish_ticket,
                                      Vec* acquired_resources) {
  pthread_mutex_lock(&cook->mtx);

  FIFOQueueNode* node;
  FIFOQueueNode* best_node = nullptr;
  double best_score = -1;
  for (node = cook->dish_tickets.head; node != nullptr; node = node->next) {
    DishTicket* dish_ticket = node->value;
    double score = kitchen_calculate_contention_score(
        &cook->restaurant->kitchen, dish_ticket);

    if (score > best_score) {
      best_score = score;
      best_node = node;
    }
  }
  pthread_mutex_unlock(&cook->mtx);

  // Queue empty.
  if (best_node == nullptr) {
    return RESULT_OK;
  }

  DishTicket* dish_ticket = best_node->value;
  Result result = kitchen_try_get_resources(&cook->restaurant->kitchen,
                                            &dish_ticket->dish->requirements,
                                            acquired_resources);

  if (result == RESULT_OK) {
    pthread_mutex_lock(&cook->mtx);
    queue_remove_at(&cook->dish_tickets, best_node);
    *selected_dish_ticket = dish_ticket;
    cook->queued_time -= dish_ticket->dish->cook_time;
    cook->queued_price -= dish_ticket->dish->price;
    pthread_mutex_unlock(&cook->mtx);
  }

  return result;
}

static void cook_increase_resources_dirtiness(Cook* cook,
                                              Vec* acquired_resources) {
  double total_score = 0;

  for (size_t i = 0; i < acquired_resources->length; i++) {
    KitchenResource** kitchen_resource_ref = vec_at(acquired_resources, i);
    KitchenResource* kitchen_resource = *kitchen_resource_ref;

    int dirtiness = atomic_fetch_add(&kitchen_resource->dirtiness, 1);

    if (dirtiness > 0) {
      total_score -=
          pow(2, dirtiness) * log2(1 + kitchen_resource->resource->clean_time);
    }
  }

  // Lock once, not for every resource.
  pthread_mutex_lock(&cook->restaurant->mtx);
  cook->restaurant->score -= (int)ceil(total_score);
  pthread_mutex_unlock(&cook->restaurant->mtx);
}

static Result cook_wash_dirty_resources(Cook* cook, Vec* acquired_resources) {
  Vec dirty_resources;
  vec_init(&dirty_resources, sizeof(KitchenResource*));

  // Worst case scenario but acquired_resources is not big and makes error
  // handling easier.
  Result result = vec_reserve(&dirty_resources, acquired_resources->length);
  if (result != RESULT_OK) {
    return result;
  }

  pthread_mutex_lock(&cook->mtx);
  double avg_queue_urgency = cook->queued_price / fmax(cook->queued_time, 1.0);
  pthread_mutex_unlock(&cook->mtx);
  int waiting = atomic_load(&cook->restaurant->sink.waiting);

  for (size_t i = 0; i < acquired_resources->length; i++) {
    KitchenResource** kitchen_resource_ref = vec_at(acquired_resources, i);
    KitchenResource* kitchen_resource = *kitchen_resource_ref;

    double next_dirty_cost = pow(2, kitchen_resource->dirtiness) *
                             log2(1 + kitchen_resource->resource->clean_time);

    int wash_delay = kitchen_resource->resource->clean_time *
                     (waiting + dirty_resources.length + 1);

    double clean_cost = wash_delay * avg_queue_urgency;

    if (next_dirty_cost > clean_cost) {
      // Cannot fail since we reserved enough space above.
      vec_push(&dirty_resources, kitchen_resource_ref);
    } else {
      atomic_store(&kitchen_resource->available, true);
    }
  }

  vec_drop(acquired_resources, nullptr);

  // Wash all dirty resources and free them right away.
  sink_wash_all(&cook->restaurant->sink, &dirty_resources);
  vec_drop(&dirty_resources, nullptr);

  return RESULT_OK;
}

static void cook_drop_resources_dirty(Vec* acquired) {
  for (size_t i = 0; i < acquired->length; i++) {
    KitchenResource** kitchen_resource_ref = vec_at(acquired, i);
    KitchenResource* kitchen_resource = *kitchen_resource_ref;
    // Resource still exclusively held by the cook, so it's safe to modify
    // the value without affecting calculations in other places.
    kitchen_resource->dirtiness = 1;
    atomic_store(&kitchen_resource->available, true);
  }

  vec_drop(acquired, nullptr);
}

static Result cook_thread(void* void_cook) {
  Cook* cook = void_cook;

  while (true) {
    sem_wait(&cook->sem);

    if (restaurant_is_closing(cook->restaurant)) {
      break;
    }

    DishTicket* dish_ticket = nullptr;
    Vec acquired_resources;
    Result result =
        cook_select_next_ticket(cook, &dish_ticket, &acquired_resources);

    if (result == RESULT_REQUIREMENTS_UNAVAILABLE) {
      restaurant_time_wait(cook->restaurant, 1);
      sem_post(&cook->sem);
      continue;
    }

    if (result != RESULT_OK) {
      free(dish_ticket);
      goto cleanup;
    }

    if (dish_ticket == nullptr) {
      continue;
    }

    // Cook the dish.
    restaurant_time_wait(cook->restaurant, dish_ticket->dish->cook_time);

    // Give it to customer right away to maximize score.
    result = waiter_assign(dish_ticket->waiter, dish_ticket);
    if (result != RESULT_OK) {
      free(dish_ticket);
      goto cleanup;
    }

    cook_increase_resources_dirtiness(cook, &acquired_resources);

    result = cook_wash_dirty_resources(cook, &acquired_resources);

  cleanup:
    if (result != RESULT_OK) {
      // Cook dead, cannot wash plates.
      cook_drop_resources_dirty(&acquired_resources);
      return result;
    }
  }

  return RESULT_OK;
}

Result cook_init(Cook* cook, Restaurant* restaurant) {
  cook->restaurant = restaurant;
  cook->queued_time = 0;
  cook->queued_price = 0;
  queue_init(&cook->dish_tickets, sizeof(DishTicket));
  pthread_mutex_init(&cook->mtx, nullptr);
  sem_init(&cook->sem, 0, 0);
  return thread_init(&cook->tid, cook_thread, cook);
}

Result cook_assign(Cook* cook, DishTicket* dish_ticket) {
  pthread_mutex_lock(&cook->mtx);
  Result result = queue_push_last(&cook->dish_tickets, dish_ticket);
  if (result == RESULT_OK) {
    cook->queued_time += dish_ticket->dish->cook_time;
    cook->queued_price += dish_ticket->dish->price;
    sem_post(&cook->sem);
  }
  pthread_mutex_unlock(&cook->mtx);
  return result;
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

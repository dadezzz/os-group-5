#include "status.h"

#include <pthread.h>
#include <stdio.h>

#include "state/kitchen.h"
#include "state/restaurant.h"
#include "threads/cook.h"
#include "vec.h"

void status_print(Restaurant* restaurant, bool extended_print) {
  pthread_mutex_lock(&restaurant->mtx);
  printf("\n=== RESTAURANT STATUS ===\n");
  printf("Current score:   %d\n", restaurant->score);
  printf("Customers:\n");
  printf("--- currently in restaurant:   %d\n", restaurant->present_customers);
  printf("--- left unserved:   %d\n", restaurant->left_unserved_customers);
  printf("--- progress (spawned / total):  %d / %d\n",
         restaurant->spawned_customers, restaurant->config->total_customers);
  int percentage = ((int)restaurant->spawned_customers * 100) /
                   (int)restaurant->config->total_customers;
  printf("--- progress percentage:   %d%%\n", percentage);

  pthread_mutex_unlock(&restaurant->mtx);

  if (extended_print) {
    printf("Lenght of cooks' dishes queues:\n");
    for (size_t i = 0; i < restaurant->cooks.length; i++) {
      Cook* cook = vec_at(&restaurant->cooks, i);
      pthread_mutex_lock(&cook->mtx);
      printf("--- cook %lu:   %lu\n", i, cook->dish_tickets.length);
      pthread_mutex_unlock(&cook->mtx);
    }

    printf("Current availability of kitchen resources:\n");
    for (size_t i = 0; i < restaurant->kitchen.resources.length; i++) {
      KitchenResource* kitchen_resource =
          vec_at(&restaurant->kitchen.resources, i);

      printf("--- %s:   %b\n", kitchen_resource->resource->name,
             atomic_load(&kitchen_resource->available));
    }
  }
}

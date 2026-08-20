#include "restaurant.h"

#include <semaphore.h>

void restaurant_init(Restaurant* restaurant, int seats) {
  sem_init(&restaurant->seats, 0, seats);
}

void restaurant_drop(Restaurant* restaurant) {
  sem_destroy(&restaurant->seats);
}

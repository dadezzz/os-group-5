#ifndef RESTAURANT_H
#define RESTAURANT_H

#include <semaphore.h>

typedef struct Restaurant {
  sem_t seats;
} Restaurant;

void restaurant_init(Restaurant* restaurant, int seats);

void restaurant_drop(Restaurant* restaurant);

#endif

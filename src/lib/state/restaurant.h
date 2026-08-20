#ifndef RESTAURANT_H
#define RESTAURANT_H

typedef struct {
  int seats;
} Restaurant;

void restaurant_init(Restaurant* restaurant, int seats);

void restaurant_drop(Restaurant* restaurant);

#endif

#include "sink.h"

#include <pthread.h>

#include "../timer.h"
#include "kitchen.h"
#include "restaurant.h"

void sink_wash(Sink* sink, KitchenResource* resource) {
  pthread_mutex_lock(&sink->mtx);

  pthread_mutex_lock(&sink->restaurant->kitchen.mtx);
  resource->available = false;
  pthread_mutex_unlock(&sink->restaurant->kitchen.mtx);

  timer_wait(sink->restaurant->timer, resource->resource->clean_time);

  pthread_mutex_lock(&sink->restaurant->kitchen.mtx);
  resource->available = true;
  resource->dirtiness = 0;
  pthread_mutex_unlock(&sink->restaurant->kitchen.mtx);

  pthread_mutex_unlock(&sink->mtx);
}

void sink_init(Sink* sink, Restaurant* restaurant) {
  sink->restaurant = restaurant;
  pthread_mutex_init(&sink->mtx, nullptr);
}

void sink_drop(Sink* sink) {
  if (sink == nullptr) {
    return;
  }

  pthread_mutex_destroy(&sink->mtx);
}

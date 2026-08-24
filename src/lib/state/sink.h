#ifndef SINK_H
#define SINK_H

#include <pthread.h>
#include <stdatomic.h>

#include "kitchen.h"

typedef struct Restaurant Restaurant;

typedef struct Sink {
  pthread_mutex_t mtx;
  Restaurant* restaurant;
  atomic_int waiting;
} Sink;

void sink_wash(Sink* sink, KitchenResource* resource);

void sink_init(Sink* sink, Restaurant* restaurant);

void sink_drop(Sink* sink);

#endif

#ifndef SINK_H
#define SINK_H

#include <semaphore.h>
#include <stdatomic.h>

#include "../vec.h"

typedef struct Restaurant Restaurant;

typedef struct Sink {
  Restaurant* restaurant;
  atomic_int waiting;
  sem_t locked;
} Sink;

void sink_wash_all(Sink* sink, Vec* dirty_resources);

void sink_init(Sink* sink, Restaurant* restaurant);

void sink_drop(Sink* sink);

#endif

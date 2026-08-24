#ifndef SINK_H
#define SINK_H

#include <pthread.h>

#include "../timer.h"
#include "kitchen.h"

typedef struct Sink {
  pthread_mutex_t mtx;
  Timer* timer;
} Sink;

void sink_wash(Sink* sink, KitchenResource* resource);

void sink_init(Sink* sink, Timer* timer);

void sink_drop(Sink* sink);

#endif

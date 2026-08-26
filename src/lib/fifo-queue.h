#ifndef FIFO_QUEUE_H
#define FIFO_QUEUE_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "result.h"

typedef struct FIFOQueueNode {
  void* value;
  struct FIFOQueueNode* next;
} FIFOQueueNode;

typedef struct FIFOQueue {
  size_t value_size;
  size_t length;
  FIFOQueueNode* head;
  FIFOQueueNode* tail;
} FIFOQueue;

void queue_init(FIFOQueue* queue, size_t value_size);

bool queue_is_empty(FIFOQueue* queue);

void* queue_remove_at(FIFOQueue* queue, FIFOQueueNode* node);

Result queue_push_last(FIFOQueue* queue, const void* value);

Result queue_push_last_allocated(FIFOQueue* queue, void* value);

void* queue_pop_first(FIFOQueue* queue);

void queue_drop(FIFOQueue* queue, void (*drop_cb)(void*));

#endif

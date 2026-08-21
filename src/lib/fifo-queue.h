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
  size_t item_size;
  FIFOQueueNode* head;
  FIFOQueueNode* tail;
} FIFOQueue;

void queue_init(FIFOQueue* queue, size_t item_size);

bool queue_is_empty(FIFOQueue* queue);

Result queue_push(FIFOQueue* queue, const void* item);

void* queue_pop(FIFOQueue* queue);

void queue_drop(FIFOQueue* queue, void (*drop_cb)(void*));

#endif

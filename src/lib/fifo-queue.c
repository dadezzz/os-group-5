#include "fifo-queue.h"

#include <stdlib.h>
#include <string.h>

#include "result.h"

void queue_init(FIFOQueue* queue, size_t value_size) {
  queue->value_size = value_size;
  queue->head = nullptr;
  queue->tail = nullptr;
}

bool queue_is_empty(FIFOQueue* queue) {
  return queue->head == nullptr;
}

Result queue_push(FIFOQueue* queue, const void* value) {
  FIFOQueueNode* new_node = malloc(sizeof(FIFOQueueNode));
  if (new_node == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  new_node->value = malloc(queue->value_size);
  if (new_node->value == nullptr) {
    free(new_node);
    return RESULT_OUT_OF_MEMORY;
  }

  memcpy(new_node->value, value, queue->value_size);
  new_node->next = nullptr;

  if (queue_is_empty(queue)) {
    queue->head = new_node;
  } else {
    queue->tail->next = new_node;
  }

  queue->tail = new_node;

  return RESULT_OK;
}

// Like queue_push but re-uses the memory of the passed value.
Result queue_push_allocated(FIFOQueue* queue, void* value) {
  FIFOQueueNode* new_node = malloc(sizeof(FIFOQueueNode));
  if (new_node == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  new_node->value = value;
  new_node->next = nullptr;

  if (queue_is_empty(queue)) {
    queue->head = new_node;
  } else {
    queue->tail->next = new_node;
  }

  queue->tail = new_node;

  return RESULT_OK;
}

void* queue_pop(FIFOQueue* queue) {
  if (queue->head == nullptr) {
    return nullptr;
  }

  FIFOQueueNode* node = queue->head;
  queue->head = node->next;
  void* value = node->value;

  // If we popped the last item set tail to null.
  if (node == queue->tail) {
    queue->tail = nullptr;
  }

  free(node);
  return value;
}

void queue_drop(FIFOQueue* queue, void (*drop_cb)(void*)) {
  while (!queue_is_empty(queue)) {
    void* item = (queue_pop(queue));

    if (drop_cb != nullptr) {
      drop_cb(item);
    }

    free(item);
  }
}

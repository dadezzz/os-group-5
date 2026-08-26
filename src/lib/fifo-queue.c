#include "fifo-queue.h"

#include <stdlib.h>
#include <string.h>

#include "result.h"

void queue_init(FIFOQueue* queue, size_t value_size) {
  queue->value_size = value_size;
  queue->length = 0;
  queue->head = nullptr;
  queue->tail = nullptr;
}

bool queue_is_empty(FIFOQueue* queue) {
  return queue->head == nullptr;
}

void* queue_remove_at(FIFOQueue* queue, FIFOQueueNode* node) {
  if (queue_is_empty(queue)) {
    return nullptr;
  }

  FIFOQueueNode* parent = nullptr;
  FIFOQueueNode* current = queue->head;
  while (current != nullptr && current != node) {
    parent = current;
    current = current->next;
  }

  // Didn't find node in the list.
  if (current == nullptr) {
    return nullptr;
  }

  if (parent != nullptr) {
    parent->next = current->next;
  } else {
    queue->head = current->next;
  }

  void* value = current->value;

  // Update queue tail.
  if (queue->tail == current) {
    queue->tail = parent;
  }

  free(current);

  queue->length--;

  return value;
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

  // Setting value to nullptr allows to allocate and then initialize in place.
  if (value != nullptr) {
    memcpy(new_node->value, value, queue->value_size);
  }

  new_node->next = nullptr;

  if (queue_is_empty(queue)) {
    queue->head = new_node;
  } else {
    queue->tail->next = new_node;
  }

  queue->tail = new_node;
  queue->length++;

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
  queue->length++;

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
  if (queue_is_empty(queue)) {
    queue->tail = nullptr;
  }

  free(node);
  queue->length--;

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

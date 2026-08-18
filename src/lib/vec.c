#include "vec.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "result.h"

void vec_init(Vec* vec, size_t item_size) {
  // Initialize to 1 so that we can simply double it.
  vec->capacity = 1;
  vec->items = nullptr;
  vec->item_size = item_size;
  vec->length = 0;
}

Result vec_push(Vec* vec, void* item) {
  // Capacity is always a power of 2, to not waste too much memory on small
  // vectors and to not require too many allocations on big vectors.
  size_t new_capacity = vec->capacity;
  if (vec->length + 1 > vec->capacity) {
    new_capacity *= 2;
  }

  void* new_items = realloc(vec->items, vec->item_size * new_capacity);

  if (new_items == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  vec->items = new_items;
  vec->capacity = new_capacity;

  memcpy(vec->items + (vec->length * vec->item_size), item, vec->item_size);

  vec->length += 1;

  return RESULT_OK;
}

void* vec_at(Vec* vec, size_t i) {
  return vec->items + i * vec->item_size;
}

// Drops items held by the vector and resets lenght and capacity.
// The vector is still usable after this, with the same item_size.
void vec_drop(Vec* vec) {
  if (vec == nullptr) {
    return;
  }

  free(vec->items);
  vec->items = nullptr;
  // Again, to 1 or doubling doesn't work.
  vec->capacity = 1;
  vec->length = 0;
}

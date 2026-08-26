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

Result vec_reserve(Vec* vec, size_t length) {
  // Capacity is always a power of 2, to not waste too much memory on small
  // vectors and to not require too many allocations on big vectors.
  while (vec->capacity < length) {
    vec->capacity *= 2;
  }

  void* new_items = realloc(vec->items, vec->capacity * vec->item_size);

  if (new_items == nullptr) {
    return RESULT_OUT_OF_MEMORY;
  }

  vec->items = new_items;
  return RESULT_OK;
}

void* vec_at(Vec* vec, size_t i) {
  return vec->items + i * vec->item_size;
}

Result vec_push(Vec* vec, const void* item) {
  Result result = vec_reserve(vec, vec->length + 1);
  if (result != RESULT_OK) {
    return result;
  }

  memcpy(vec_at(vec, vec->length), item, vec->item_size);
  vec->length += 1;

  return RESULT_OK;
}

// Drops items held by the vector and resets lenght and capacity.
// The vector is still usable after this, with the same item_size.
void vec_drop(Vec* vec, void (*drop_cb)(void*)) {
  if (vec == nullptr) {
    return;
  }

  if (drop_cb != nullptr) {
    for (size_t i = 0; i < vec->length; ++i) {
      void* item = vec_at(vec, i);
      drop_cb(item);
    }
  }

  free(vec->items);
  vec->items = nullptr;
  // Again, to 1 or doubling doesn't work.
  vec->capacity = 1;
  vec->length = 0;
}

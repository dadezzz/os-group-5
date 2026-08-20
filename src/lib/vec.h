#ifndef VEC_H
#define VEC_H

#include <stddef.h>

#include "result.h"

typedef struct {
  void* items;
  size_t item_size;
  size_t length;
  size_t capacity;
} Vec;

void vec_init(Vec* vec, size_t item_size);

Result vec_push(Vec* vec, const void* item);

void* vec_at(Vec* vec, size_t i);

void vec_drop(Vec* vec, void (*drop_cb)(void*));

#endif

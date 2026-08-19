#include "str.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "vec.h"

// PERF: the use of vec even when dest is nullptr is for simplicity but could
// be overridden with a simple counter to avoid allocations.

size_t read_str_until_char(const char* src, char** dest, char pattern) {
  Vec dest_vec;
  vec_init(&dest_vec, sizeof(char));

  while (src[dest_vec.length] != pattern && src[dest_vec.length] != '\0') {
    vec_push(&dest_vec, &src[dest_vec.length]);
  }

  // Push a null terminator at the end to make dest a valid C string.
  char terminator = '\0';
  vec_push(&dest_vec, &terminator);

  // Avoid null pointer dereference.
  if (dest != nullptr) {
    *dest = dest_vec.items;
  } else {
    free(dest_vec.items);
  }

  return dest_vec.length - 1;
}

size_t read_file_until_char(FILE* file, char** dest, char pattern) {
  Vec dest_vec;
  vec_init(&dest_vec, sizeof(char));

  for (int c = getc(file); c != pattern && c != EOF; c = getc(file)) {
    char cc = (char)c;
    vec_push(&dest_vec, &cc);
  }

  // Push a null terminator at the end to make dest a valid C string.
  char terminator = '\0';
  vec_push(&dest_vec, &terminator);

  // Avoid null pointer dereference.
  if (dest != nullptr) {
    *dest = dest_vec.items;
  } else {
    free(dest_vec.items);
  }

  return dest_vec.length - 1;
}

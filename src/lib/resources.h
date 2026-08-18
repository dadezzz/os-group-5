#ifndef RESOURCES_H
#define RESOURCES_H

#include <stddef.h>

#include "result.h"

typedef struct {
  char* name;
  int quantity;
  int clean_time;
} Resource;

typedef struct {
  size_t length;
  Resource* items;
} Resources;

Resources* resources_new();

Result resources_load(const char* file_path, Resources* resources);

void resources_drop(Resources* resources);

#endif

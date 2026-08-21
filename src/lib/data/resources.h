#ifndef RESOURCES_H
#define RESOURCES_H

#include "../result.h"
#include "../vec.h"

typedef struct Resource {
  char* name;
  int quantity;
  int clean_time;
} Resource;

Result resources_load(const char* file_path, Vec* resources);

void resource_drop(void* arg);

#endif

#ifndef DISHES_H
#define DISHES_H

#include "../../result.h"
#include "../../vec.h"

typedef struct Dish {
  char* name;
  int price;
  int cook_time;
  // Vec of Requirement.
  Vec requirements;
} Dish;

Result dishes_load(const char* file_path, Vec* dishes, Vec* resources);

void dish_drop(void* arg);

#endif

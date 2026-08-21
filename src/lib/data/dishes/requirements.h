#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include "../../result.h"
#include "../../vec.h"
#include "../resources.h"

typedef struct {
  Resource* resource;
  int quantity;
} Requirement;

Result requirements_load(const char* requirements_str, Vec* requirements, Vec* resources);

void requirement_drop(void* arg);

#endif

#ifndef REQUIREMENTS_H
#define REQUIREMENTS_H

#include "../result.h"
#include "../vec.h"

typedef struct {
  char* name;
  int quantity;
} Requirement;

Result requirements_load(const char* requirements_str, Vec* requirements);

void requirement_drop(void* arg);

#endif

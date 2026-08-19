#include "resources.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "result.h"
#include "str.h"
#include "vec.h"

static Result parse_resource(const char* resource_str, Resource* resource) {
  size_t name_bytes = read_str_until_char(resource_str, &resource->name, ',');

  if (name_bytes == 0) {
    return RESULT_RESOURCES_FILE_INVALID;
  }

  resource_str += name_bytes + 1;

  char* quantity_str = nullptr;
  size_t quantity_bytes = read_str_until_char(resource_str, &quantity_str, ',');

  if (quantity_bytes == 0) {
    free(quantity_str);
    return RESULT_RESOURCES_FILE_INVALID;
  }

  resource_str += quantity_bytes + 1;

  resource->quantity = atoi(quantity_str);
  free(quantity_str);

  char* clean_time_str = nullptr;
  size_t clean_time_bytes =
      read_str_until_char(resource_str, &clean_time_str, ',');

  if (clean_time_bytes == 0) {
    free(clean_time_str);
    return RESULT_RESOURCES_FILE_INVALID;
  }

  resource->clean_time = atoi(clean_time_str);
  free(clean_time_str);

  return RESULT_OK;
}

static Result parse_file(FILE* file, Vec* resources) {
  // Skip reading the first line (CSV header).
  read_file_until_char(file, nullptr, '\n');

  // Parse each subsequent line.
  while (true) {
    char* line_str = nullptr;
    size_t line_bytes = read_file_until_char(file, &line_str, '\n');

    if (line_bytes == 0 && feof(file)) {
      free(line_str);
      break;
    }

    // At least 5 bytes required to make a valid 3 field CSV line.
    if (line_bytes < 5) {
      free(line_str);
      continue;
    }

    Resource new_resource = {};
    Result result = parse_resource(line_str, &new_resource);
    free(line_str);
    if (result != RESULT_OK) {
      free(new_resource.name);
      return result;
    }

    result = vec_push(resources, &new_resource);
    if (result != RESULT_OK) {
      free(new_resource.name);
      return result;
    }
  }

  return RESULT_OK;
}

Result resources_load(const char* file_path, Vec* resources) {
  vec_init(resources, sizeof(Resource));

  FILE* file = fopen(file_path, "r");
  if (file == nullptr) {
    return RESULT_RESOURCES_FILE_NOT_OPENED;
  }

  Result result = parse_file(file, resources);
  fclose(file);
  if (result != RESULT_OK) {
    return result;
  }

  if (resources->length == 0) {
    return RESULT_RESOURCES_FILE_EMPTY;
  }

  return RESULT_OK;
}

void resources_drop(Vec* resources) {
  if (resources == nullptr) {
    return;
  }

  for (size_t i = 0; i < resources->length; ++i) {
    Resource* resource = vec_at(resources, i);
    free(resource->name);
  }

  vec_drop(resources);
}

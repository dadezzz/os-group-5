#include "resources.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "result.h"

Resources* resources_new() {
  return malloc(sizeof(Resources));
}

Result resources_load(const char* file_path, Resources* resources) {
  resources->items = nullptr;
  resources->length = 0;

  FILE* file = fopen(file_path, "r");
  if (file == nullptr) {
    return RESULT_RESOURCES_FILE_NOT_OPENED;
  }

  // Skip reading the first line (CSV header).
  fscanf(file, "%*[^\n]\n");

  // Capacity is the size that gets allocated. It doubles expoentially to avoid
  // expensive re-allocations.
  size_t new_items_capacity = 1;

  // Parse each subsequent line.
  while (true) {
    int quantity = 0;
    int clean_time = 0;
    char* name = nullptr;

    int read = fscanf(file, "%m[^,],%d,%d\n", &name, &quantity, &clean_time);

    // Break at end of file.
    if (read == EOF) {
      break;
    }

    // Return error if line didn't have all 3 sections.
    if (read != 3) {
      free(name);
      fclose(file);
      return RESULT_RESOURCES_FILE_INVALID;
    }

    // Double the capacity each time. To avoid allocating on every cycle.
    if (resources->length + 1 > new_items_capacity) {
      new_items_capacity *= 2;
    }

    Resource* new_items =
        realloc(resources->items, sizeof(Resource) * new_items_capacity);

    // Check that realloc was successfull.
    if (new_items == nullptr) {
      free(name);
      fclose(file);
      return RESULT_OUT_OF_MEMORY;
    }

    resources->items = new_items;

    // Parse CSV line and load values in the Resource.
    Resource* resource = &resources->items[resources->length];
    resource->name = name;
    resource->clean_time = clean_time;
    resource->quantity = quantity;

    resources->length += 1;
  }

  fclose(file);

  if (resources->length == 0) {
    return RESULT_RESOURCES_FILE_EMPTY;
  }

  return RESULT_OK;
}

void resources_drop(Resources* resources) {
  // Avoid null pointer dereferences.
  if (!resources) {
    return;
  }

  for (size_t i = 0; i < resources->length; ++i) {
    free(resources->items[i].name);
  }

  free(resources->items);
  free(resources);
}

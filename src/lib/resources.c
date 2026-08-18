#include "resources.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "result.h"
#include "vec.h"

Result resources_load(const char* file_path, Vec* resources) {
  vec_init(resources, sizeof(Resource));

  FILE* file = fopen(file_path, "r");
  if (file == nullptr) {
    return RESULT_RESOURCES_FILE_NOT_OPENED;
  }

  // Skip reading the first line (CSV header).
  fscanf(file, "%*[^\n]\n");

  // Parse each subsequent line.
  while (true) {
    Resource new_resource = {};

    int read = fscanf(file, "%m[^,],%d,%d\n", &new_resource.name,
                      &new_resource.quantity, &new_resource.clean_time);

    // Break at end of file.
    if (read == EOF) {
      break;
    }

    // Return error if line didn't have all 3 sections.
    if (read != 3) {
      free(new_resource.name);
      fclose(file);
      return RESULT_RESOURCES_FILE_INVALID;
    }

    // Check that realloc was successfull.
    if (vec_push(resources, &new_resource) != RESULT_OK) {
      free(new_resource.name);
      fclose(file);
      return RESULT_OUT_OF_MEMORY;
    }
  }

  fclose(file);

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

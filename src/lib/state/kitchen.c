#include "kitchen.h"

#include <math.h>
#include <stdatomic.h>
#include <stddef.h>

#include "../data/dishes/requirements.h"
#include "../data/resources.h"
#include "../result.h"
#include "../vec.h"
#include "dish-ticket.h"

// acquired is used remember exactly which resources were acquired by the
// cook.
Result kitchen_try_get_resources(Kitchen* kitchen,
                                 Vec* requirements,  // Vec<Requirement>
                                 Vec* acquired       // Vec<KitchenResource*>
) {
  vec_init(acquired, sizeof(KitchenResource*));

  bool all_available = true;
  Result result = RESULT_OK;

  for (size_t r = 0; r < requirements->length; ++r) {
    Requirement* requirement = vec_at(requirements, r);
    int available_count = 0;

    for (size_t k = 0; available_count < requirement->quantity &&
                       k < kitchen->resources.length;
         k++) {
      KitchenResource* kitchen_resource = vec_at(&kitchen->resources, k);
      if (kitchen_resource->resource != requirement->resource) {
        continue;
      }

      bool available = true;
      atomic_compare_exchange_strong(&kitchen_resource->available, &available,
                                     false);

      if (available) {
        // Store a ref to the kitchen resource, so that we can modify its
        // value from inside acquired.
        result = vec_push(acquired, &kitchen_resource);
        if (result == RESULT_OK) {
          ++available_count;
        } else {
          goto cleanup;
        }
      }
    }

    if (available_count < requirement->quantity) {
      all_available = false;
      break;
    }
  }

  if (!all_available) {
    result = RESULT_REQUIREMENTS_UNAVAILABLE;
  }

cleanup:
  if (result != RESULT_OK) {
    for (size_t i = 0; i < acquired->length; ++i) {
      KitchenResource** kitchen_resource_ref = vec_at(acquired, i);
      KitchenResource* kitchen_resource = *kitchen_resource_ref;
      atomic_store(&kitchen_resource->available, false);
    }
    vec_drop(acquired, nullptr);
  }

  return result;
}

double kitchen_calculate_contention_score(Kitchen* kitchen,
                                          DishTicket* ticket) {
  double score = 0;
  double available_ratio_sum = 0;
  int requirement_count = 0;

  for (size_t r = 0; r < ticket->dish->requirements.length; r++) {
    Requirement* requirement = vec_at(&ticket->dish->requirements, r);

    // Get availability for this resource type
    int available = 0;
    int total = 0;

    for (size_t i = 0; i < kitchen->resources.length; ++i) {
      KitchenResource* kitchen_resource = vec_at(&kitchen->resources, i);

      if (kitchen_resource->resource == requirement->resource) {
        ++total;

        if (atomic_load(&kitchen_resource->available)) {
          ++available;
        }
      }
    }

    if (total == 0) {
      return -INFINITY;
    }

    double ratio = (double)available / total;
    available_ratio_sum += ratio;
    requirement_count++;

    // Penalize if not enough available
    if (available < requirement->quantity) {
      score -= 1000;
    }
  }

  score += available_ratio_sum / requirement_count;
  return score;
}

Result kitchen_init(Kitchen* kitchen, Vec* resources  // Vec<Resource>
) {
  vec_init(&kitchen->resources, sizeof(KitchenResource));

  for (size_t i = 0; i < resources->length; i++) {
    Resource* resource = vec_at(resources, i);

    for (int j = 0; j < resource->quantity; j++) {
      KitchenResource kitchen_resource;
      kitchen_resource.resource = resource;
      kitchen_resource.dirtiness = 0;
      atomic_init(&kitchen_resource.available, true);

      Result result = vec_push(&kitchen->resources, &kitchen_resource);
      if (result != RESULT_OK) {
        return result;
      }
    }
  }

  return RESULT_OK;
}

void kitchen_drop(Kitchen* kitchen) {
  if (kitchen == nullptr) {
    return;
  }

  vec_drop(&kitchen->resources, nullptr);
}

#ifndef SIGUSR1_H
#define SIGUSR1_H

#include "result.h"

Result sigusr1_register_handler();

bool sigusr1_get_raised();

void sigusr1_set_raised(bool value);

#endif

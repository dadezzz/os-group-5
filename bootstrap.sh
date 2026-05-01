#!/bin/sh

# TODO:
#
# - read --env-file param (if present);
# - check that .env exists;
# - read env vars from .env;
# - override env vars with cli params (if present);
# - validate env vars type (positive int, string, etc.);
# - validate that the menu and resources files specified in the respective variables are in the correct format;
# - start the restaurant binary with the provided args
#
# Params:
#
# - NUM_COOKS: positive int
# - NUM_WAITERS: positive int
# - MAX_CUSTOMERS: positive int
# - TOTAL_CUSTOMERS: positive int
# - MENU_FILE: string (path)
# - RESOURCES_FILE: string (path)
# - GAME_SPEED: positive float
# - RANDOM_SEED: positive int

#!/usr/bin/env bash

# TODO: check that this ends up being the correct path during submission.
source ./src/lib/result.sh

POSITIVE_INT_REGEX="[1-9][0-9]*"
IS_POSITIVE_INT_REGEX="^$POSITIVE_INT_REGEX\$"
function is_positive_int {
    [[ "$1" =~ $IS_POSITIVE_INT_REGEX ]]
}

function is_positive_float {
    # Second condition requires that at least one digit is different from 0.
    [[ "$1" =~ ^[0-9]*(\.[0-9]+)?$ ]] && [[ "$1" =~ [1-9] ]]
}

function file_exists {
    [[ -n "$1" ]] && [[ -f "$1" ]]
}

MENU_CSV_REQUIREMENT_REGEX="[^,]+(:$POSITIVE_INT_REGEX)?"
MENU_CSV_LINE_REGEX="^[^,]+,$POSITIVE_INT_REGEX,$POSITIVE_INT_REGEX,$MENU_CSV_REQUIREMENT_REGEX(;$MENU_CSV_REQUIREMENT_REGEX)*\$"
function menu_file_valid {
    local first_line=true

    # Read last line even if missing a newline.
    while IFS= read -r line || [[ -n "$line" ]]; do
        if [[ $first_line = true ]]; then
            if [[ "$line" != "name,price,time,requirements" ]]; then
                return 1
            fi

            first_line=false
        else
            if [[ ! "$line" =~ $MENU_CSV_LINE_REGEX ]]; then
                return 1
            fi
        fi
    done < "$1"

    return 0
}

RESOURCES_CSV_LINE_REGEX="^[^,]+,$POSITIVE_INT_REGEX,$POSITIVE_INT_REGEX\$"
function resources_file_valid {
    local first_line=true

    # Read last line even if missing a newline.
    while IFS= read -r line || [[ -n "$line" ]]; do
        if [[ $first_line = true ]]; then
            if [[ "$line" != "resource,quantity,clean_time" ]]; then
                return 1
            fi

            first_line=false
        else
            if [[ ! "$line" =~ $RESOURCES_CSV_LINE_REGEX ]]; then
                return 1
            fi
        fi
    done < "$1"

    return 0
}

# By default env file is .env
ENV_FILE=.env

# Read args
for arg in "$@" ; do
    case $arg in
        --env-file=*)
            ENV_FILE="${arg#*=}"
            ;;
        --num-cooks=*)
            NUM_COOKS_ARG="${arg#*=}"
            if ! is_positive_int "$NUM_COOKS_ARG" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        --num-waiters=*)
            NUM_WAITERS_ARG="${arg#*=}"
            if ! is_positive_int "$NUM_WAITERS_ARG"; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        --max-customers=*)
            MAX_CUSTOMERS_ARG="${arg#*=}"
            if ! is_positive_int "$MAX_CUSTOMERS_ARG" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        --total-customers=*)
            TOTAL_CUSTOMERS_ARG="${arg#*=}"
            if ! is_positive_int "$TOTAL_CUSTOMERS_ARG" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        --menu-file=*)
            MENU_FILE_ARG="${arg#*=}"
            if ! file_exists "$MENU_FILE_ARG" ; then
                exit $RESULT_DISHES_FILE_NOT_OPENED
            fi
            if ! menu_file_valid "$MENU_FILE_ARG" ; then
                exit $RESULT_DISHES_FILE_INVALID
            fi
            ;;
        --resources-file=*)
            RESOURCES_FILE_ARG="${arg#*=}"
            if ! file_exists "$RESOURCES_FILE_ARG" ; then
                exit $RESULT_RESOURCES_FILE_NOT_OPENED
            fi
            if ! resources_file_valid "$RESOURCES_FILE_ARG" ; then
                exit $RESULT_RESOURCES_FILE_INVALID
            fi
            ;;
        --game-speed=*)
            GAME_SPEED_ARG="${arg#*=}"
            if ! is_positive_float "$GAME_SPEED_ARG" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        --random-seed=*)
            RANDOM_SEED_ARG="${arg#*=}"
            if ! is_positive_int "$RANDOM_SEED_ARG" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        *)
            exit $RESULT_CONFIG_INVALID_PARAMETER
            ;;
    esac
done

if ! file_exists "$ENV_FILE" ; then
    exit $RESULT_CONFIG_FILE_NOT_OPENED
fi

# Read last line even if missing a newline.
while read -r line || [[ -n "$line" ]]; do
    # Skip empty lines.
    if [[ -z "$line" ]]; then
        continue
    fi

    case "$line" in
        NUM_COOKS=*)
            NUM_COOKS_ENV="${line#*=}"
            if ! is_positive_int "$NUM_COOKS_ENV" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        NUM_WAITERS=*)
            NUM_WAITERS_ENV="${line#*=}"
            if ! is_positive_int "$NUM_WAITERS_ENV"; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        MAX_CUSTOMERS=*)
            MAX_CUSTOMERS_ENV="${line#*=}"
            if ! is_positive_int "$MAX_CUSTOMERS_ENV" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        TOTAL_CUSTOMERS=*)
            TOTAL_CUSTOMERS_ENV="${line#*=}"
            if ! is_positive_int "$TOTAL_CUSTOMERS_ENV" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        MENU_FILE=*)
            MENU_FILE_ENV="${line#*=}"
            if ! file_exists "$MENU_FILE_ENV" ; then
                exit $RESULT_DISHES_FILE_NOT_OPENED
            fi
            if ! menu_file_valid "$MENU_FILE_ENV" ; then
                exit $RESULT_DISHES_FILE_INVALID
            fi
            ;;
        RESOURCES_FILE=*)
            RESOURCES_FILE_ENV="${line#*=}"
            if ! file_exists "$RESOURCES_FILE_ENV" ; then
                exit $RESULT_RESOURCES_FILE_NOT_OPENED
            fi
            if ! resources_file_valid "$RESOURCES_FILE_ENV" ; then
                exit $RESULT_RESOURCES_FILE_INVALID
            fi
            ;;
        GAME_SPEED=*)
            GAME_SPEED_ENV="${line#*=}"
            if ! is_positive_float "$GAME_SPEED_ENV" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        RANDOM_SEED=*)
            RANDOM_SEED_ENV="${line#*=}"
            if ! is_positive_int "$RANDOM_SEED_ENV" ; then
                exit $RESULT_CONFIG_INVALID_VALUE
            fi
            ;;
        \#*) # Allow comments in env file.
            ;;
        *)
            exit $RESULT_CONFIG_INVALID_PARAMETER
            ;;
    esac
done < "$ENV_FILE"

# Use cli argument with preceence over the one in the env file.
export NUM_COOKS="${NUM_COOKS_ARG:-$NUM_COOKS_ENV}"
export NUM_WAITERS="${NUM_WAITERS_ARG:-$NUM_WAITERS_ENV}"
export MAX_CUSTOMERS="${MAX_CUSTOMERS_ARG:-$MAX_CUSTOMERS_ENV}"
export TOTAL_CUSTOMERS="${TOTAL_CUSTOMERS_ARG:-$TOTAL_CUSTOMERS_ENV}"
export MENU_FILE="${MENU_FILE_ARG:-$MENU_FILE_ENV}"
export RESOURCES_FILE="${RESOURCES_FILE_ARG:-$RESOURCES_FILE_ENV}"
export GAME_SPEED="${GAME_SPEED_ARG:-$GAME_SPEED_ENV}"
export RANDOM_SEED="${RANDOM_SEED_ARG:-$RANDOM_SEED_ENV}"

# TODO: also check the path of this in the final submission.
exec ./build/restaurant

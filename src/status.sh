#!/usr/bin/env bash

source ./src/lib/result.sh

PID_PATH=/tmp/restaurant.pid

if [ ! -f "$PID_PATH" ] ; then
  echo "$PID_PATH doesn't exist. Restaurant executable needs to be running."
  exit $RESULT_PID_FILE_NOT_FOUND;
fi

PID=$(cat $PID_PATH)

if [ ! -d "/proc/$PID" ] || [ "$(stat -c '%u' "/proc/$PID" 2>/dev/null)" != "$(id -u)" ]; then
  echo "The file $PID_PATH exists but doesn't point to a process owned by the current user."
  exit $RESULT_PID_FILE_NOT_OPENED;
fi

kill -USR1 "$PID"

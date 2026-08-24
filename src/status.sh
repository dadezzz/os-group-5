#!/bin/sh

PID_PATH=/tmp/restaurant.pid

if [ ! -f "$PID_PATH" ] ; then
  echo "$PID_PATH doesn't exist. Restaurant executable needs to be running."
  exit 1
fi

PID=$(cat $PID_PATH)

if [ ! $(ps -A -o uid,pid | grep -E "^\s$(id -u)\s$PID" ) ] ; then
  echo "The file $PID_PATH exists but doesn't point to a process owned by the current user."
  exit 1
fi

kill -USR1 "$PID"

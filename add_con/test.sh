#!/bin/bash

psql mydb -c "select shared_reset();" -o /dev/null

echo -en "four sessions add concurrenly..."

psql mydb -c "select shared_add() from generate_series(1, 1000000);" -o /dev/null &
psql mydb -c "select shared_add() from generate_series(1, 1000000);" -o /dev/null &
psql mydb -c "select shared_add() from generate_series(1, 1000000);" -o /dev/null &
psql mydb -c "select shared_add() from generate_series(1, 1000000);" -o /dev/null &

wait

actual=$(psql mydb -t -c "select shared_get();" | egrep -o '[0-9]+')
expected=4000000

if [ $actual -ne $expected ]; then
  echo -en "\033[31;1mfailed\n\033[0m"
else
  echo -en "\033[32;1mok\n\033[0m"
fi
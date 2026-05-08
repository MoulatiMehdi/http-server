#!/bin/bash

HOST="127.0.0.1"
PORT="8080"

COUNT="${1:-10000}"

for ((i=1; i<=COUNT; i++))
do
{
    echo "Connection $i"

    (
        sleep 1
    ) | telnet "$HOST" "$PORT" > /dev/null 2>&1

} &
done

wait

echo "Done"

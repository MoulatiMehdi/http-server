#!/bin/bash

# Wipe environment
for var in $(env | cut -d= -f1); do
    unset "$var"
done

# Write response exactly
printf "Status: 200 OK\r\n"
printf "Content-Type: text/plain\r\n"
printf "Content-Length: 13\r\n"
printf "\r\n"
printf "Hello, world!"

#!/bin/bash

# Ignore everything from environment
unset $(env | cut -d= -f1)

# Output valid HTTP response
printf "Status: 200 OK\r\n"
printf "Content-Type: text/html\r\n"
printf "\r\n"
printf "<html><body><h1>Hello from CGI</h1></body></html>\n"

#!/bin/bash

# Number of times to run the telnet client
NUM_RUNS=10000

# Messages to send
messages=("Hello, Server!" "How are you?" "This is a test." "Thanks for the response!" "Goodbye!")

# Loop to execute telnet multiple times
for ((i=0; i<NUM_RUNS; i++)); do
    {
        # Start the telnet client on localhost port 8080
        echo "Connecting to telnet server..."
        (
            for message in "${messages[@]}"; do
                echo "$message"  # Send message to the server
                sleep 1          # Wait for a second
            done
            echo "quit"  # Use "quit" to exit the telnet session
        ) | telnet localhost 8080
    } &
done

wait  # Wait for all background processes to finish

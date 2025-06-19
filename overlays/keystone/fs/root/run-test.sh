#!/bin/sh

# Initialize variables
DURATION="1h"

# Parse command-line arguments
while [[ $# -gt 0 ]]; do
    case "$1" in
        -t)
            if [[ -n "$2" ]]; then
                DURATION="$2"
                shift 2
            else
                echo "Error: -t option requires an argument."
                exit 1
            fi
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "Starting iperf3 server..."

iperf3 -s > iperf3.log &
iperf3_pid=$!

echo "iperf3 server started. Start the iperf client before continuing."

read -p "Press enter to continue..."


echo "Running stress-ng and cyclictest for a duration of $DURATION..."

stress-ng --all 1 -t $DURATION -x netlink-task,swap --log-file stress-ng.log &
stressng_pid=$!

#cyclictest -vm -i100 -p99 -t --duration=$DURATION > cyclictest.log &
/usr/share/keystone/examples/latency.ke -- -vm -i1000000 -p99 -t --duration=30s > cyclictest.log &

cyclictest_pid=$!

wait "$cyclictest_pid"
kill "$stressng_pid"
kill "$iperf3_pid"

echo "Done"

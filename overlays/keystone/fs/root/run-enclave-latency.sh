#!/bin/sh

# Initialize variables
DURATION="1h"

declare -a PIDS=()

cleanup() {
    echo "Cleaning up processes..."
    for pid in "${PIDS[@]}"; do
        if kill -0 "$pid" 2>/dev/null; then
            echo "Killing process $pid"
            kill "$pid" 2>/dev/null
        fi
    done
    exit
}

trap cleanup SIGINT SIGTERM EXIT

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

stress-ng --all 1 -t $DURATION -x netlink-task,swap --log-file stress-ng.log > /dev/null &

PIDS+=($!)

/usr/share/keystone/examples/latency.ke -- -a0 -vm -i1000000 -p99 -t --duration=$DURATION > cyclictest0.log &
/usr/share/keystone/examples/latency.ke -- -a1 -vm -i1000000 -p99 -t --duration=$DURATION > cyclictest1.log &
/usr/share/keystone/examples/latency.ke -- -a2 -vm -i1000000 -p99 -t --duration=$DURATION > cyclictest2.log &
/usr/share/keystone/examples/latency.ke -- -a3 -vm -i1000000 -p99 -t --duration=$DURATION > cyclictest3.log &
cyclictest_pid+=$!


wait "$cyclictest_pid"
cleanup

echo "Done"

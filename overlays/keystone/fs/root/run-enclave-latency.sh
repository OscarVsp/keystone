#!/bin/sh

# Initialize variables
DURATION="1h"
SKIP_LOAD=0
NUM_THREADS=4
NUM_LOOP=0
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
        -l)
            if [[ -n "$2" ]]; then
                NUM_LOOP="$2"
                shift 2
            else
                echo "Error: -l option requires an argument."
                exit 1
            fi
            ;;
        -n)
            if [[ -n "$2" ]] && [[ "$2" =~ ^[1-4]$ ]]; then
                NUM_THREADS="$2"
                shift 2
            else
                echo "Error: -n option requires a number between 1 and 4."
                exit 1
            fi
            ;;
        -d)
            SKIP_LOAD=1
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [-t DURATION] [-d] [-n NUM_THREADS]"
            echo "  -t DURATION     Set test duration (default: 1h)"
            echo "  -d              Skip iperf3 and stress-ng, run only latency tests"
            echo "  -n NUM_THREADS  Number of latency test threads to run (1-4, default: 4)"
            exit 1
            ;;
    esac
done
# Conditionally run iperf3 and stress-ng
if [ "$SKIP_LOAD" -eq 0 ]; then
    echo "Starting iperf3 server..."
    iperf3 -s > iperf3.log &
    PIDS+=($!)
    
    echo "iperf3 server started. Start the iperf client before continuing."
    read -p "Press enter to continue..."
    
    echo "Running stress-ng and cyclictest for a duration of $DURATION..."
    stress-ng --all 1 -t $DURATION -x netlink-task,swap --log-file stress-ng.log > /dev/null &
    PIDS+=($!)
else
    echo "Skipping iperf3 and stress-ng (latency tests only mode)"
fi

# Build CPU affinity string (e.g., "0-2" for 3 threads, "0-3" for 4 threads)
CPU_AFFINITY="0-$((NUM_THREADS - 1))"

# Run latency test with N threads
echo "Running cyclictest with $NUM_THREADS thread(s) on CPUs $CPU_AFFINITY for duration of $DURATION with a max loop of $NUM_LOOP"
/usr/share/keystone/examples/cyclictest.ke -- -l $NUM_LOOP -a$CPU_AFFINITY -vm -i2000000  -d10 -p99 -t $NUM_THREADS --duration=$DURATION > cyclictest.log &
cyclictest_pid=$!


wait "$cyclictest_pid"
cleanup

echo "Done"

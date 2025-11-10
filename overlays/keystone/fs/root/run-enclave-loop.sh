#!/bin/bash

while true; do
    echo "running enclave"
    /usr/share/keystone/examples/loop-task.ke &
    pid=$!
    wait "$pid"
    echo "waiting"
    sleep 5
done
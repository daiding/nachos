#!/bin/bash
make -j4
./nachos 7 20 1000 50 -rs 666 > test.log &
sleep 10s
pkill -9 nachos
for occupancy in {0..20}
do
    grep Occupancy:\ $occupancy\  test.log -E | wc -l
done
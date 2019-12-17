#!/bin/bash

threadnum=2
nodenum=2

make -j4

for errornum in {1..5}
do
    echo "[Nachos] nachos 2 $threadnum $nodenum $errornum"
    ./nachos 2 $threadnum $nodenum $errornum
    echo
done

echo "[Nachos] nachos 2 $threadnum $nodenum 6 -M (Without Switches, with big mutex)"
./nachos 2 $threadnum $nodenum 6 -M
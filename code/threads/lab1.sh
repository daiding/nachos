#!/bin/bash

threadnum=2
nodenum=2

make -j4

for errornum in {1..5}
do
    echo "[Nachos] nachos 1 $threadnum $nodenum $errornum"
    ./nachos 1 $threadnum $nodenum $errornum
    echo
done

echo "[Nachos] nachos 1 $threadnum $nodenum 6 (Without Switches)"
./nachos 1 $threadnum $nodenum 6
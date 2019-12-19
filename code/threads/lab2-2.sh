#!/bin/bash

threadnum=2
nodenum=2

make -j4

for errornum in {4..5}
do
    echo "[Nachos] nachos 3 $threadnum $nodenum $errornum"
    ./nachos 3 $threadnum $nodenum $errornum
    echo
done
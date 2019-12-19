import random
import os
import time
import subprocess

def main():
    test_times = 100
    test_maximum = 1000
    test_delay = 1
    success = 0
    for i in range(test_times):
        elevatorNum = random.randint(1,test_maximum)
        riderNum = random.randint(1,test_maximum)
        floorsNum = random.randint(1,test_maximum)
        process = subprocess.Popen(f"./nachos 7 {elevatorNum} {riderNum} {floorsNum}".split(' '),
                                    stdout=subprocess.DEVNULL)
        time.sleep(test_delay)
        if process.poll() != None:
            print(f"[FAILED {i}] Process Exited at Test({elevatorNum}, {riderNum}, {floorsNum})")
            break
        else:
            process.kill()
            print(f"[SUCCESS {i}]./nachos 7 {elevatorNum} {riderNum} {floorsNum}  ")
            success += 1
    if(success == test_times):
        print("** Test success **")

main()
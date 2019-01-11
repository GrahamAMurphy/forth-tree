#!/bin/sh
# Download speed test; "<reboot> execute" to start

echo ": 2+ 2 + ;"
echo ": 2- 2 - ;"
echo ": 4+ 4 + ;"
echo ": 4- 4 - ;"
squish $FTROOT/lib/motor2.fr

echo "motor"
motor -d 70000 $HOME/forth/chips/scip/32bits/amba/xilinx/xil32bootforhigh.mem

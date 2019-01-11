#!/bin/sh
# Compile Forth for CRISM DPU

forth auto
motor -s1 crisfor.mem >crisfor.abs
bin2prom -p 0 -e 42a <crisfor.mem >crisfore.mem
motor -s1 crisfore.mem >crisfore.abs

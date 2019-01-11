#!/bin/sh
# Compile Forth for CONTOUR DPU

forth auto
motor -s1 cxxfor.mem >cxxfor.abs
bin2prom -p 0 -e 42a <cxxfor.mem >cxxfore.mem
motor -s1 cxxfore.mem >cxxfore.abs

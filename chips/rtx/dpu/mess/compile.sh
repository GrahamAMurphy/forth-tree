#!/bin/sh
# Compile Forth for CONTOUR DPU

forth auto
motor -s1 messfor.mem >messfor.abs
bin2prom -p 0 -e 42a <messfor.mem >messfore.mem
motor -s1 messfore.mem >messfore.abs

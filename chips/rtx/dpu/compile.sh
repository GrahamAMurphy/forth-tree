#!/bin/sh

# Compile Forth system
forth auto

# Generate loads for NEAR DPUs
#  fordpu.alp: alpha load version to RAM page 1 over RS232
dpuload -t 0 -d 10000 fordpu.mem
mv fordpu.mem.alp fordpu.alp
#  fordpue.int: Intel hex version for GSEOS load over 1553 to EEPROM
dpuload -t 2 -f all -s 100 -d 0 -e 42a fordpu.mem
mv fordpu.mem.int fordpue.int		# (must be ftp'ed onto PC disk)

#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <strofor.mem >strofore.mem		# for saving in EEPROM

# Generate S-records
motor -s1 strofor.mem >strofor.abs
motor -s1 strofore.mem >strofore.abs

# Generate SMF (Simple Memory Format).  Can be used in Xilinx BRAMs.
smf strofor.mem >strofor.smf

#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <cordfor.mem >cordfore.mem		# for saving in EEPROM

# Generate S-records
# Note: if downloaded into ROM-less boot logic S-record loader, starts at
# address 0 (this is the default address in the final S-record).
motor -s1 cordfor.mem >cordfor.abs
motor -s1 cordfore.mem >cordfore.abs

# Generate SMF (Simple Memory Format).  Can be used in Xilinx BRAMs.
smf cordfor.mem >cordfor.smf

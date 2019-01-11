#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto

# Generate S-records
# Note: RAM is in page 1.  Also, -g option will boot Forth when this
# is downloaded into ROM-less boot logic S-record loader.
motor -d 10000 -g 10000 cordipfor.mem >cordipfor.abs

# Generate SMF (Simple Memory Format).  Can be used in Xilinx BRAMs.
smf cordipfor.mem >cordipfor.smf

#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <ravanfor.mem >ravanfore.mem	# for saving in EEPROM

# Generate S-records
motor -s1 ravanfor.mem >ravanfor.abs
motor -s1 ravanfore.mem >ravanfore.abs

# Generate SMF (Simple Memory Format)
smf ravanfor.mem >ravanfor.smf

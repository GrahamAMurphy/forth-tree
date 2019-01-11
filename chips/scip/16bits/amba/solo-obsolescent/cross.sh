#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <solofor.mem >solofore.mem		# for saving in EEPROM

# Generate S-records
motor -s1 solofor.mem >solofor.abs
motor -s1 solofore.mem >solofore.abs

# Generate SMF (Simple Memory Format)
smf solofor.mem >solofor.smf

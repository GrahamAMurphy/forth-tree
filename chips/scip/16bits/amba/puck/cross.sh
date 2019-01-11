#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <puckfor.mem >puckfore.mem		# for saving in EEPROM

# Generate S-records
motor -s1 puckfor.mem >puckfor.abs
motor -s1 puckfore.mem >puckfore.abs

# Generate SMF (Simple Memory Format)
smf puckfor.mem >puckfor.smf

#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
bin2prom -p 0 -e 210 <com1for.mem >com1fore.mem		# for saving in EEPROM

# Generate S-records
motor -s1 com1for.mem >com1for.abs
motor -s1 com1fore.mem >com1fore.abs

# Generate SMF (Simple Memory Format)
smf com1for.mem >com1for.smf

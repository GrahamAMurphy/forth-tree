#!/bin/sh
# Cross-compile

# Compile Forth.
forth auto					# for running in RAM (any page)
formatmem.sh vslfor.mem >vslfor.smf		# for vsim test bench
bin2prom -p 0 -e 210 <vslfor.mem >vslfore.mem	# for saving in EEPROM

# Compile Forth with bootboot.  Forth is automatically copied from ROM page 0
# to RAM page 0.
forth auto-boot
formatmem.sh vslbootfor.mem >vslbootfor.smf	# for vsim test bench
motor vslbootfor.mem >vslboot.abs		# for burning ROMs

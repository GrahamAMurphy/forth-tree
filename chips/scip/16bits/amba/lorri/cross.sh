#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
formatmem.sh lorrfor.mem >lorrfor.smf
bin2prom -p 0 -e 210 <lorrfor.mem >lorrfore.mem	# for saving in EEPROM

# Compile Forth with bootboot.  Forth is automatically copied from ROM page 0
# to RAM page 0.
forth auto-boot
formatmem.sh lorrbootfor.mem >lorrbootfor.smf

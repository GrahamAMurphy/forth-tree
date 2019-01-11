#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
formatmem.sh trinityfor.mem >trinityfor.smf
bin2prom -p 0 -e 210 <trinityfor.mem >trinityfore.mem	# for saving in EEPROM

# Compile Forth with bootboot.  Forth is automatically copied from ROM page 0
# to RAM page 0.
forth auto-boot
formatmem.sh trinitybootfor.mem >trinitybootfor.smf

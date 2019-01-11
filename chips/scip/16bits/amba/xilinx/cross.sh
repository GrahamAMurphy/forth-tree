#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
formatmem.sh xil16for.mem >xil16for.smf

# Compile Forth with bootboot.  Forth is automatically copied from ROM page 0
# to RAM page 0.
forth auto-boot
formatmem.sh xil16bootfor.mem >xil16bootfor.smf

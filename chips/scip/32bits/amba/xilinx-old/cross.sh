#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto
formatmem.sh xil32for.mem >xil32for.smf

# Compile Forth with bootboot.  Forth is automatically copied from ROM address 0
# to RAM address 0.
forth auto-boot
formatmem.sh xil32bootfor.mem >xil32bootfor.smf

# Compile Forth with boottest.  Forth is automatically copied from ROM address 0
# to high RAM.
forth auto-boot-high
formatmem.sh xil32bootforhigh.mem >xil32bootforhigh.smf

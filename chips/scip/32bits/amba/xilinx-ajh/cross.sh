#!/bin/sh
# Cross-compile

# Compile Forth with bootboot.  Forth is automatically copied from ROM address 0
# to RAM address 0.
forth auto
formatmem.sh xilajh.mem >xilajh.smf

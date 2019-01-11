#!/bin/sh
# Generate alpha loadable Forth system RAM.
# (Assumes that boot ROM is using RAM page 1).
nearload -d 10000 newforth >newforth.alp
# Generate alpha loadable Forth system for EEPROM file 0.
nearload -f 0 -s 100 -d 0 -e 42a newforth > forthg0.alp
# Generate alpha loadable Forth system for EEPROM file 1 (for now).
#nearload -f 1 -s 8000 -d 0 -e 42a newforth > forthg0.alp

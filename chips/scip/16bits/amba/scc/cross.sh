#!/bin/sh
# Cross-compile

# Compile Forth.  Can run in any page.
forth auto

# Generate S-records
motor -s1 sccfor.mem >sccfor.abs

# Generate SMF (Simple Memory Format)
smf sccfor.mem >sccfor.smf

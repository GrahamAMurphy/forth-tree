#!/bin/sh
# Compile MESSENGER generic instrument programs.  Generate an motorola file for
# the application in autoboot format (for saving in EEPROM).  File should be
# loaded, then copied from page 0 to EEPROM.
# Usage: compile.sh [dpu|epu|all]

echo "Compiling EPU application ..."

# Setup correct environment
t=`cat $MESS/bin/epu-path`
FTPATH=". ../lib16 `eval echo $t`"
export FTPATH

# Compile
VER=1
FILE=epuapp$VER
forth auto-epu
#mv epuapp.mem $FILE.mem
#motor -d 0 $FILE.mem >$FILE.abs
#bin2prom -p 0 -e 42a <$FILE.mem >${FILE}e.mem
#motor -d 0 ${FILE}e.mem >${FILE}e.abs

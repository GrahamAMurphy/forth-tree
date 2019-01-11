#!/bin/sh
# Usage: down-epu.sh [ ... ]

# Setup correct environment
t=`cat $MESS/bin/epu-path`
FTPATH="../lib16 `eval echo $t`"
export FTPATH

# Download EPU board simulation
squish messepusim.fr
squish -l rtxprims.fr

# Download common code (some customization files are plucked from current dir)
squish generic.fr
loadcommonepu $*

# Application-specific code
squish macro-dflt.fr
squish monitor-dflt.fr
squish 1hz-proc.fr

# Initialization
squish main-epu.fr

# Testing ...
# squish -l tests/core-cmds.fr
squish testload.fr

# Complete
echo "7 emit"
echo ".( Type 'go' to start program)"
echo "quit"

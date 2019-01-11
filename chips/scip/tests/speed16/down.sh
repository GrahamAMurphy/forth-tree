#!/bin/sh
# Download EEPROM test

echo ": between 1+ within ;"

squish -l rtxcross.fr
squish -l far2.fr
squish -l memory1.fr
squish -l motor2.fr

echo "download"
motor -d 8000 speed.mem

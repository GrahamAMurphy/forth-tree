#!/bin/sh
# Generate Forth for Neutron Spectrometer Processor

# Compile
forth auto

#Generate an intel file for loading and running in RAM.
intel -p 7 forneut >forneut.int

# Generate an intel file in autoboot format (for saving in EEPROM).  File
# should be loaded, then copied from page 7 to EEPROM.
bin2prom -p 7 -e 42a <forneut | intel -p 7 >forneute.int

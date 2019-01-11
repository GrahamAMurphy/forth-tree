#!/bin/sh
# Generate test programs in Simple Memory Format (SMF).

forth auto-hi
formatmem.sh hi.mem >hi.smf

# self-test program in initialized RAM.  Used in Frisc5-instance and
# xilinx-lorri (Xilinx) testbench.
forth auto-selftest
formatmem.sh selftest.mem >selftest.smf

# self-test program for xilinx-lorri (Actel) testbench.
forth auto-selftest1
formatmem.sh selftest1.mem >selftest1.smf

# self-test program for xilinx-lorri (Actel) testbench.
forth auto-selftest2
formatmem.sh selftest2.mem >selftest2.smf

# VSLIT self-test program for testbench
forth auto-selftestvslit
formatmem.sh selftestvslit.mem >selftestvslit.smf

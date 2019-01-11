#!/bin/sh
# Compile selected (or all) RTX Forth systems.

# default to all if no arguments
if [ $# -eq 0 ]; then
	targets="mpe dpu ctp aiu nlr frisc3 vme"
else
	targets=$*
fi

# compile selected targets
for target in $targets
do
	case $target in

	mpe)
		(cd $FTROOT/rtx/mpe
		 echo "Compiling MPE systems ..."
		 forth auto		# multi-tasked system
		 motor mpeforth >mpeforth.mot
		 forth autosing		# single task system 
		 motor mpesing >mpesing.mot
	);;

	dpu|msi)
		if [ $target != "msi" ]
		then
			(cd $FTROOT/rtx/dpu
			 echo "Compiling DPU system ..."
			 forth auto
			 dpuload -t 0 -f 0 -s 100 -d 0 -e 42a dpu
			 dpuload -t 2 -f 0 -s 100 -d 0 -e 42a dpu
			)
		fi
		(cd $FTROOT/rtx/dpu/msi
		 echo "Compiling DPU (MSI specific) system ..."
		 forth auto
		 dpuload -t 2 -f 0 -s 100 -d 0 -e 42a dpumsi
		);;

	ctp)
		(cd $FTROOT/rtx/ctp
		 echo "Compiling CTP system ..."
		 forth auto
		 motor ctpforth >ctpforth.mot
		);;

	aiu)
		(cd $FTROOT/rtx/aiu
		 echo "Compiling AIU system ..."
		 forth auto
		 motor aiuforth >aiuforth.mot
		);;

	nlr)
		(cd $FTROOT/rtx/nlr
		 echo "Compiling NLR system ..."
		 forth auto
		 motor -d 1000 nlrforth >nlrforth.mot
		);;

	frisc3)
		(cd $FTROOT/Frisc3
		 echo "Compiling FRISC3 system ..."
		 forth auto
		);;

	vme)
		(cd $FTROOT/Frisc3/Vme
		 echo "Compiling FRISC3 VME system ..."
		 forth auto
		 motor vmeforth >vmeforth.mot
		);;

	esac
done

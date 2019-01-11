#!/bin/sh
# Check or gild Forth systems.
# Usage: check [check|gild]	# check is default

type=${1:-"check"}

cd $FTROOT/rtx/mpe
if [ $type = "gild" ]; then
	echo "Gilding MPE systems ..."
	cp mpeforth mpeforth.gld
	cp mpesing mpesing.gld
else
	echo "Checking MPE systems ... "
	cmp mpeforth.gld mpeforth
	cmp mpesing.gld mpesing
fi

cd $FTROOT/rtx/dpu
if [ $type = "gild" ]; then
	echo "Gilding DPU system ..."
	cp dpu dpu.gld
else
	echo "Checking DPU system ..."
	cmp dpu.gld dpu
fi

cd $FTROOT/rtx/ctp
if [ $type = "gild" ]; then
	echo "Gilding CTP system ..."
	cp ctpforth ctpforth.gld
else
	echo "Checking CTP system ..."
	cmp ctpforth.gld ctpforth
fi

cd $FTROOT/rtx/aiu
if [ $type = "gild" ]; then
	echo "Gilding AIU system ..."
	cp aiuforth aiuforth.gld
else
	echo "Checking AIU system ..."
	cmp aiuforth.gld aiuforth
fi

cd $FTROOT/rtx/nlr
# TBD
if [ $type = "gild" ]; then
	echo "Gilding NLR system ..."
fi

cd $FTROOT/Frisc3
if [ $type = "gild" ]; then
	echo "Gilding FRISC3 system ..."
	cp newforth newforth.gld
else
	echo "Checking FRISC3 system ..."
	cmp newforth.gld newforth
fi

cd $FTROOT/Frisc3/Vme
if [ $type = "gild" ]; then
	echo "Gilding Frisc3 VME ..."
	cp vmeforth vmeforth.gld
else
	echo "Checking Frisc3 VME ..."
	cmp vmeforth.gld vmeforth
fi

# Frisc4 is temporarily out of order ...
#cd $FTROOT/Frisc4
#echo "Processing Frisc4 ..."
#rm -f newforth
#forth auto
#if [ $type = "gild" ]
#then
#	cmp newforth newforth.gld
#else
#	gild newforth
#fi

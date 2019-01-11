#!/bin/sh
for trace in freja topex traps oops
do
	echo $trace
	/usr/5bin/echo "\t64\t128\t256\t512\t1024\t2048\t4096\t8192\t16384"
	for assoc in 1 2 4 8 16
	do
		(echo $assoc
		for size in 64 128 256 512 1024 2048 4096 8192 16384
		do
			isacache -a $assoc -c 100000 -l 1000000 -s $size \
							${trace}trace
		done) | tr '\012' '\011'
		echo
	done
	echo
done

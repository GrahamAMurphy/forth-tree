#!/bin/sh
/usr/5bin/echo "\t64\t128\t256\t512\t1024\t2048\t4096\t8192\t16384"
for trace in freja topex traps oops
do
	(echo $trace
	for size in 64 128 256 512 1024 2048 4096 8192 16384
	do
		idmcache -m -c 100000 -l 1000000 -s $size ${trace}trace
	done) | tr '\012' '\011'
	echo
done

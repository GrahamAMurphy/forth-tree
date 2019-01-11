#!/bin/sh
trace=traps
for thresh in 0 1 2
do
	(echo $trace $thresh
	for size in 64 128 256 512 1024 2048 4096 8192 16384
	do
		idmexcache -t $thresh -c 100000 -l 1000000 -s $size \
						${trace}trace
	done) | tr '\012' '\011'
	echo
done

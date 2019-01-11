#!/bin/sh
# Measure 1024-cell direct-mapped caches with different line sizes
for i in 1 2 4
do
	echo $i
	for trace in freja topex traps oops
	do
		echo $trace
		idmcache -m -v -c 100000 -l 1000000 -s 1024 -b $i \
			${trace}trace | tr ':' '\011'
	done
done

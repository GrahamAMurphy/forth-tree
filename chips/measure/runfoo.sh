#!/bin/sh
# Measure 1024-cell caches
for trace in freja topex traps oops
do
	for linesize in 1 2 4 8
	do
		(echo -n "$trace(I,$linesize):$linesize:"
		idmcache -v -c 100000 -l 1000000 -s 1024 -b $linesize \
			${trace}trace) | tr ':' '\011'
	done
	for linesize in 1 2 4 8
	do
		(echo -n "$trace(M,$linesize):$linesize:"
		idmcache -v -m -c 100000 -l 1000000 -s 1024 -b $linesize \
			${trace}trace) | tr ':' '\011'
	done
done

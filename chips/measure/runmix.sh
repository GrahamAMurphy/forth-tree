#!/bin/sh
# Measure 1024 cell direct mapped caches, first with instructions only,
# then with instructions and data.
for trace in freja topex traps oops
do
	echo $trace
	idmcache -v -c 100000 -l 1000000 -s 1024 ${trace}trace \
		| tr ':' '\011'
done
for trace in freja topex traps oops
do
	echo $trace
	idmcache -m -v -c 100000 -l 1000000 -s 1024 ${trace}trace \
		| tr ':' '\011'
done

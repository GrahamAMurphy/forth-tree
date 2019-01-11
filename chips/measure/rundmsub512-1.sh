#!/bin/sh
(for trace in freja topex traps oops
do
	echo -n "$trace-i:512:1:"
	dmsubcache -v -c 100000 -l 1000000 -s 512 ${trace}trace
	for w in a m n
	do
		echo -n "$trace-m$w:512:1:"
		dmsubcache -v -m$w -c 100000 -l 1000000 -s 512 ${trace}trace
	done
done) | tr ':' '\011' >dmsub512-1results
